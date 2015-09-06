using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SysProtocolDef;
using ConnProtocolDef;

namespace ServerManagementTool
{
    static public class ProtocolNoList
    {
        static private Dictionary<int, Type> ProtocolNoDict;

        static public Dictionary<int, Type> GetProtocolNoDict()
        {
            if( null != ProtocolNoDict )
                return ProtocolNoDict;

            ProtocolNoDict = new Dictionary<int, Type>();

            ProtocolNoDict[REQ_LOGIN.No] = typeof( REQ_LOGIN );
            ProtocolNoDict[ACK_LOGIN.No] = typeof( ACK_LOGIN );
            ProtocolNoDict[CMD_HEARTBEAT.No] = typeof( CMD_HEARTBEAT );
            // TODO: 필요한 프로토콜을 여기에 추가

            return ProtocolNoDict;
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
            // TODO: 이하 코드를 본 프로젝트에 맞게 수정할 것(변수명도 적절히 변경할 것)
            object ProtocolObj = null;
            Type ProtocolType = null;
            IProtocolBase TargetPacket = null;

            Dictionary<int, Type> Protocols = ProtocolNoList.GetProtocolNoDict();
            if( null == Protocols )
                return;

            if( Protocols.TryGetValue( Packet.Protocol, out ProtocolType ) )
            {
                ProtocolObj = Activator.CreateInstance( ProtocolType );

                if( ProtocolObj is IProtocolBase )
                {
                    TargetPacket = ProtocolObj as IProtocolBase;
                    TargetPacket.Read( Packet );
                }
                else
                {
                    // TODO: Unknown pakcet 출력
                    //       IProtocolBase 계열과 맞지 않는 ProtocolType형 객체(ProtocolObj)다.
                }

                // TODO: 패킷을 꺼내서 처리할 곳에서 m_PackQueue를 구현한다.
                lock( m_PackQueue )
                {
                    m_PackQueue.Enqueue( TargetPacket );
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
