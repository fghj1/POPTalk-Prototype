using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using ServerManagementTool;

// 시스템 관련 프로토콜 정의
namespace SysProtocolDef
{
    #region CMD_HEARTBEAT - 10000
    public class CMD_HEARTBEAT : IProtocolBase
    {
        public const int No = 10000;
        public UInt64 To = 0;
        public UInt64 From = 0;

        public UInt64 GeneralKey = 0;

        int GetProtocolNo()
        {
            return No;
        }

        UInt64 GetTo()
        {
            return To;
        }

        UInt64 GetFrom()
        {
            return From;
        }

        bool Read( IPacketBase UnitPacket )
        {
            // General Key
            if( false == UnitPacket.Read( out GeneralKey ) )
                return false;

            // TODO: 로그인 성공했을 때 서버로부터 받는 키 값이 제대로 읽혀진 것인지 확인할 것

            return true;
        }

        bool Write( IPacketBase UnitPacket )
        {
            // General Key
            if( false == UnitPacket.Write( GeneralKey ) )
                return false;

            // TODO: 로그인 성공했을 때 서버로부터 받는 키 값이 제대로 쓰여진 것인지 확인할 것

            return true;
        }
    }
    #endregion
}
