using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using ServerManagementTool;

// 연결 관련 프로토콜 정의
namespace ConnProtocolDef
{
    #region REQ_LOGIN - 20000
    public class REQ_LOGIN : IProtocolBase
    {
        public const int No = 20000;
        public UInt64 To = 0;
        public UInt64 From = 0;

        public string ID = string.Empty;
        public string Password = string.Empty;

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
            int Length = 0;
            char[] Temp = null;

            // ID
            UnitPacket.Read( out Length );
            if( false == UnitPacket.Read( out Temp, Length ) )
                return false;

            ID = new string( Temp );

            Length = 0;
            Temp = null;

            // Password
            UnitPacket.Read( out Length );
            if( false == UnitPacket.Read( out Temp, Length ) )
                return false;

            Password = new string( Temp );

            // TODO: ID, Password가 제대로 읽혀진 것인지 확인할 것

            return true;
        }

        bool Write( IPacketBase UnitPacket )
        {
            // ID
            if( false == UnitPacket.Write( ID.Length ) )
                return false;

            if( false == UnitPacket.Write( ID.ToCharArray(), ID.Length ) )
                return false;

            // Password
            if( false == UnitPacket.Write( Password.Length ) )
                return false;

            if( false == UnitPacket.Write( Password.ToCharArray(), Password.Length ) )
                return false;

            // TODO: ID, Password가 제대로 쓰여진 것인지 확인할 것

            return true;
        }
    }
    #endregion

    #region ACK_LOGIN - 20001
    public class ACK_LOGIN : IProtocolBase
    {
        public const int No = 20001;
        public UInt64 To = 0;
        public UInt64 From = 0;

        public int LoginResult = 0;
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
            // Login result
            if( false == UnitPacket.Read( out LoginResult ) )
                return false;

            // General Key
            if( false == UnitPacket.Read( out GeneralKey ) )
                return false;

            // TODO: 로그인 결과와 로그인 성공했을 때 서버로부터 받는 키 값이 제대로 읽혀진 것인지 확인할 것

            return true;
        }

        bool Write( IPacketBase UnitPacket )
        {
            // Login result
            if( false == UnitPacket.Write( LoginResult ) )
                return false;

            // General Key
            if( false == UnitPacket.Write( GeneralKey ) )
                return false;

            // TODO: 로그인 결과와 로그인 성공했을 때 서버로부터 받는 키 값이 제대로 쓰여진 것인지 확인할 것

            return true;
        }
    }
    #endregion
}
