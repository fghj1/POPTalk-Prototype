using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ServerManagementTool
{
    protected class Peer
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
        }
    }
}
