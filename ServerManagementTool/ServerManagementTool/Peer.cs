using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ServerManagementTool
{
    // TODO: 패킷 프로토콜만 관리하는 클래스를 모아 별도 파일로 뺀다.
    // TODO: 패킷 프로토콜 구조 정의 클래스
    //       Matrix의 Protocol.cs / ProtocolTagMap.cs을 참고할 것

    static public class ProtocolTagList
    {
        static private Dictionary<int, Type> ProtocolTagDict;

        static public Dictionary<int, Type> GetProtocolTagDict()
        {
            if( null != ProtocolTagDict )
                return ProtocolTagDict;

            ProtocolTagDict = new Dictionary<int, Type>();

            //ProtocolTagDict[/**/] = typeof( /**/ );  // TODO: 위 패킷 프로토콜 구조 정의에서 정의된 패킷 프로토콜을 할당한다.
            //ProtocolTagDict[/**/] = typeof( /**/ );  // TODO: 위 패킷 프로토콜 구조 정의에서 정의된 패킷 프로토콜을 할당한다.

            return ProtocolTagDict;
        }
    }

    public class Peer
    {
        public void ProcessPacket( TPacket Packet )
        {
            // TODO: 이하 코드를 본 프로젝트에 맞게 수정할 것
            object ProtocolObj = null;
            Type ProtocolType = null;
            IProtocol PacketProtocol = null;  // TODO: IProtocol은 Protocol 유형의 부모이며 추상 클래스로 자식들에게 기본 함수 제시

            Dictionary<int, Type> Protocols = ProtocolTagList.GetProtocolTagDict();
            if( null == Protocols )
                return;

            if( Protocols.TryGetValue( Packet.Protocol, out ProtocolType ) )  // TOOD: TPacket도 아직 완전히 구현된 것 아니다.
            {
                ProtocolObj = Activator.CreateInstance( ProtocolType );

                if( ProtocolObj is IProtocol )
                {
                    PacketProtocol = ProtocolObj as IProtocol;
                    PacketProtocol.Read( Packet );
                }
                else
                {
                    // TODO: Unknown pakcet 출력
                    //       IProtocol 계열과 맞지 않는 ProtocolObj다.
                }

                lock( m_PackQueue )  // TODO: 처리해야 할 패킷 데이터들을 모아놓은 큐를 구현해야 한다.
                {
                    m_PackQueue.Enqueue( PacketProtocol );
                }
            }
            else
            {
                // TODO: Unknown packet 출력
                //       관리되지 않은 Packet protocol이다.
            }
        }

        // TODO: 패킷 송신 전에 패킷 송신용 버퍼에 담는 함수 구현할 것
    }
}
