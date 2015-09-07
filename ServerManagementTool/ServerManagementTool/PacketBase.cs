using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ServerManagementTool
{
    public interface IPacketBase
    {
        bool Read( out bool Value );
        bool Read( out byte Value );
        bool Read( out byte[] Value, int Length );
        bool Read( out sbyte Value );
        bool Read( out short Value );
        bool Read( out ushort Value );
        bool Read( out int Value );
        bool Read( out uint Value );
        bool Read( out long Value );
        bool Read( out ulong Value );
        bool Read( out float Value );
        bool Read( out double Value );
        bool Read( out decimal Value );
        bool Read( out char Value );
        bool Read( out char[] Value, int Length );

        bool Write( bool Value );
        bool Write( byte Value );
        bool Write( byte[] Value );
        bool Write( sbyte Value );
        bool Write( short Value );
        bool Write( ushort Value );
        bool Write( int Value );
        bool Write( uint Value );
        bool Write( long Value );
        bool Write( ulong Value );
        bool Write( float Value );
        bool Write( double Value );
        bool Write( decimal Value );
        bool Write( char Value );
        bool Write( char[] Value, int Length );
    }

    public interface IProtocolBase
    {
        int GetProtocolNo();
        UInt64 GetTo();
        UInt64 GetFrom();

        bool Read( IPacketBase UnitPacket );
        bool Write( IPacketBase UnitPacket );
    }
}
