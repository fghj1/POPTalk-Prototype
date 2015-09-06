using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace ServerManagementTool
{
    public class Network
    {
        public enum ErrorCode
        {
            // TODO: 대략적인 값을 작성한 것이므로 보강해야 한다.
            SUCCESS,
            FAILURE,
            DISCONNECT,
        }

        private Socket PeerSocket = null;

        private int NetErrNo = ( int )ErrorCode.SUCCESS;
        private ErrorCode NetworkError
        {
            get { return ( ErrorCode )Interlocked.CompareExchange( ref NetErrNo, ( int )ErrorCode.SUCCESS, ( int )ErrorCode.SUCCESS ); }
            set { Interlocked.Exchange( ref NetErrNo, ( int )value ); }
        }
        private Thread NetworkThread = null;
        private const int BuffSize = 1024 * 1024 * 2;
        private CircleStream SendBuff = new CircleStream( BuffSize );
        private CircleStream RecvBuff = new CircleStream( BuffSize );
        private byte[] TempSendBuff = new byte[BuffSize];
        private byte[] TempRecvBuff = new byte[BuffSize];

        private Peer CommonPeer = new Peer();

        public int Connect( IPAddress IPADR, int Port )
        {
            int ErrNo = 0;

            try
            {
                IPEndPoint remoteEP = new IPEndPoint( IPADR, Port );

                PeerSocket = new Socket( IPADR.AddressFamily, SocketType.Stream, ProtocolType.Tcp );
                PeerSocket.Connect( remoteEP );
                if( PeerSocket.Connected )
                {
                    PeerSocket.Blocking = false;
                    StartNetworkProcess();
                }
                else
                    ErrNo = -1;  // TODO: 이곳으로 진입하는 경우가 있을까?
            }
            catch( SocketException e )
            {
                ErrNo = e.ErrorCode;
            }

            return ErrNo;
        }

        private void StartNetworkProcess()
        {
            NetworkThread = new Thread( new ThreadStart( NetworkHandler ) );
            NetworkThread.Start();
        }

        private void NetworkHandler()
        {
            while( true )
            {
                if( ErrorCode.SUCCESS != NetworkError )
                    break;

                if( false == PeerSocket.Connected )
                {
                    NetworkError = ErrorCode.DISCONNECT;
                    break;
                }

                // TODO: 송수신 이벤트가 발생하면 각각에 대해 선택적으로 처리할 수는 없을까?
                //       아래와 같이 송수신을 처리하면 송신 처리 후 수신 처리를 하게 되고 어느 하나에 부하가 걸리면
                //       다른 하나에 대한 처리가 그만큼 늦어진다. 각각의 이벤트가 발생하면 그 때마다 처리하는 방식으로 바꾸면
                //       조금 더 반응성이 좋지 않을까?
                Sending();
                if( ErrorCode.SUCCESS != NetworkError )
                    break;

                Receiving();
                if( ErrorCode.SUCCESS != NetworkError )
                    break;

                Thread.Sleep( 1 );
            }

            PeerSocket.Close();
        }

        private void Sending()
        {
            bool IsSend = true;
            int UnitSize = 0;  // Single packet size

            while( IsSend )
            {
                lock( SendBuff )
                {
                    if( 0 < SendBuff.Length )
                    {
                        if( TempSendBuff.Length < SendBuff.Length )  // TODO: SendBuff에서 Length를 재정의하지 않았는제 문제가 될까?
                            Array.Resize<byte>( ref TempSendBuff, ( int )SendBuff.Length );

                        UnitSize = SendBuff.PeekBuffer( TempSendBuff );
                    }
                    else
                        UnitSize = 0;
                }

                if( 0 < UnitSize )
                {
                    int SentSize = 0;
                    SocketError SocketErr = SocketError.Success;

                    SentSize = PeerSocket.Send( TempSendBuff, 0, UnitSize, SocketFlags.None, out SocketErr );

                    lock( SendBuff )
                        SendBuff.Seek( ( long )SentSize, System.IO.SeekOrigin.Current );

                    switch( SocketErr )
                    {
                        case SocketError.Success:
                            break;

                        case SocketError.WouldBlock:
                        case SocketError.IOPending:
                        case SocketError.NoBufferSpaceAvailable:
                            IsSend = false;
                            break;

                        case SocketError.ConnectionReset:
                            NetworkError = ErrorCode.DISCONNECT;
                            IsSend = false;
                            break;

                        default:
                            NetworkError = ErrorCode.FAILURE;
                            IsSend = false;
                            break;
                    }
                }
                else
                    IsSend = false;
            }
        }

        private void Receiving()
        {
            int RcdSize = 0;
            SocketError SocketErr = SocketError.Success;

            do
            {
                RcdSize = PeerSocket.Receive( TempRecvBuff, 0, BuffSize, SocketFlags.None, out SocketErr );
                switch( SocketErr )
                {
                    case SocketError.Success:
                        break;

                    case SocketError.WouldBlock:
                    case SocketError.IOPending:
                    case SocketError.NoBufferSpaceAvailable:
                        break;

                    case SocketError.ConnectionReset:
                        NetworkError = ErrorCode.DISCONNECT;
                        break;

                    default:
                        NetworkError = ErrorCode.FAILURE;
                        break;
                }

                if( 0 < RcdSize )
                {
                    RecvBuff.Write( TempRecvBuff, 0, RcdSize );
                    // TODO: 예외가 발생하면 어떻게 대응할 수 있는지 C#의 예외 처리 방식을 이해하고 적절한 처리를 추가해야 한다.
                }

                ushort PacketSize = 0;
                while( RecvBuff.PeekPacketSize( out PacketSize ) )
                {
                    if( RecvBuff.Length < PacketSize )
                        break;

                    TPacket Packet = TPacket.New();

                    RecvBuff.Read( Packet.HeaderBuff, 0, Packet.HeaderBuff.Length );
                    RecvBuff.Read( Packet.DataBuff, 0, ( PacketSize - Packet.HeaderBuff.Length ) );

                    // TODO: 수신 패킷 처리기 구현
                    //       Peer.cs을 생성하고 고유 클라이언트 처리를 전담하는 수준 바로 전단계에서 필요로 하는 것들을 여기에 구현한다.
                    //       OneNote 내용(상황 정리) 참고할 것.
                    if( null != CommonPeer )
                        CommonPeer.ProcessPacket( Packet );
                    //else
                    //    // TODO: 예외 처리를 어떻게 할 것인가?
                }
            }
            while( 0 < RcdSize );
        }
    }
}
