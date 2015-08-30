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
        // TODO: 패킷 프로토콜 종류를 모은 그룹(클래스?, 구조체?) 구현
        //       수신한 데이터를 TPacket의 한 단위로 뽑아낸 후 이것을 모아 놓은 Q타입의 m_PackQueue에 담는다.
        //       수신한 패킷을 처리하는 전용 스레드에서 m_PackQueue에 담긴 패킷들을 추출하여 하나씩 처리한다.
        //       Q에 담고 이를 다시 추출하여 
        // TODO: 패킷 프로토콜 종류별 처리 함수 구현(종류별로 별도 파일 생성)
        //       이런 패킷 프로토콜 처리기를 한 곳에 수집하여 처리기가 필요할 때 꺼내서 사용할 수 있도록 한다.
        // TODO: 패킷 처리기 구현
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
