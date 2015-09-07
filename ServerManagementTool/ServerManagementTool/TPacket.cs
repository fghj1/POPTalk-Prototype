using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace ServerManagementTool
{
    public class TPacket : ObjectPool<TPacket>, IPacketBase
    {
        public byte[] HeaderBuff = new byte[PACKETHEADERSIZE];
        public byte[] DataBuff = new byte[PACKETBUFFERSIZE];

        private Stream HeaderStream = null;
        private Stream DataStream = null;

        private const int PACKETHEADERSIZE = 12;
        private const int PACKETBUFFERSIZE = 8192 * 3;

        private BinaryWriter HeaderWriter;
        private BinaryReader HeaderReader;
        private BinaryWriter DataWriter;
        private BinaryReader DataReader;

        public TPacket()
        {
            HeaderStream = new MemoryStream( HeaderBuff );
            DataStream = new MemoryStream( DataBuff );

            HeaderWriter = new BinaryWriter( HeaderStream, System.Text.Encoding.Unicode );
            HeaderReader = new BinaryReader( HeaderStream, System.Text.Encoding.Unicode );
            DataWriter = new BinaryWriter( DataStream, System.Text.Encoding.Unicode );
            DataReader = new BinaryReader( DataStream, System.Text.Encoding.Unicode );
        }

        public long SeekData( long Offset, SeekOrigin StartField )
        {
            return DataStream.Seek( Offset, StartField );
        }

        public ushort TotalSize
        {
            get { return ( ushort )( HeaderStream.Length + DataStream.Position ); }  // TODO: 의도한 값이 반환되는지 확인 필요
        }

        public byte[] ForWritingHeaderBuff
        {
            get
            {
                HeaderStream.Seek( 0, SeekOrigin.Begin );
                HeaderWriter.Write( ( ushort )TotalSize );

                return HeaderBuff;
            }
        }

        public byte[] ForReadingHeaderBuff
        {
            get { return HeaderBuff; }
        }

        public ushort Protocol
        {
            get
            {
                HeaderStream.Seek( 2, SeekOrigin.Begin );
                return HeaderReader.ReadUInt16();
            }

            set
            {
                HeaderStream.Seek( 2, SeekOrigin.Begin );
                HeaderWriter.Write( value );
            }
        }

        public byte[] Data
        {
            get { return DataBuff; }
            set { DataBuff = value; }
        }

        #region ObjectPool - Abstract function
        protected override void Reinit()
        {
            SeekData( 0, SeekOrigin.Begin );
        }
        #endregion

        #region IPacketBase - Read() group
        public bool Read( out bool Value )
        {
            Value = false;

            try
            {
                Value = DataReader.ReadBoolean();
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Read( out byte Value )
        {
            Value = 0;

            try
            {
                Value = DataReader.ReadByte();
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Read( out byte[] Value, int Length )
        {
            Value = null;

            try
            {
                Value = DataReader.ReadBytes( Length );
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Read( out sbyte Value )
        {
            Value = 0;

            try
            {
                Value = DataReader.ReadSByte();
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Read( out short Value )
        {
            Value = 0;

            try
            {
                Value = DataReader.ReadInt16();
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Read( out ushort Value )
        {
            Value = 0;

            try
            {
                Value = DataReader.ReadUInt16();
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Read( out int Value )
        {
            Value = 0;

            try
            {
                Value = DataReader.ReadInt32();
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Read( out uint Value )
        {
            Value = 0;

            try
            {
                Value = DataReader.ReadUInt32();
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Read( out long Value )
        {
            Value = 0;

            try
            {
                Value = DataReader.ReadInt64();
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Read( out ulong Value )
        {
            Value = 0;

            try
            {
                Value = DataReader.ReadUInt64();
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Read( out float Value )
        {
            Value = 0;

            try
            {
                Value = DataReader.ReadSingle();
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Read( out double Value )
        {
            Value = 0;

            try
            {
                Value = DataReader.ReadDouble();
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Read( out decimal Value )
        {
            Value = 0;

            try
            {
                Value = DataReader.ReadDecimal();
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Read( out char Value )
        {
            Value = '0';

            try
            {
                Value = DataReader.ReadChar();
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Read( out char[] Value, int Length )
        {
            Value = null;

            try
            {
                Value = DataReader.ReadChars( Length );
            }
            catch
            {
                return false;
            }

            return true;
        }
        #endregion

        #region IPacketBase - Write() group
        public bool Write( bool Value )
        {
            try
            {
                DataWriter.Write( Value );
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Write( byte Value )
        {
            try
            {
                DataWriter.Write( Value );
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Write( byte[] Value )
        {
            try
            {
                // TODO: 패킷 버퍼에 기록할 때 데이터 구조가 송수신측 모두 같은 구조를 갖는지 확인할 것
                //       예를 들어, 배열을 기록하기 전에 배열의 크기를 먼저 기록하고 내용을 기록하는 경우
                //       배열의 크기를 기록하지 않고 내용만 기록하는 경우
                DataWriter.Write( ( ushort )( Value.Length ) );
                DataWriter.Write( Value );
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Write( sbyte Value )
        {
            try
            {
                DataWriter.Write( Value );
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Write( short Value )
        {
            try
            {
                DataWriter.Write( Value );
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Write( ushort Value )
        {
            try
            {
                DataWriter.Write( Value );
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Write( int Value )
        {
            try
            {
                DataWriter.Write( Value );
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Write( uint Value )
        {
            try
            {
                DataWriter.Write( Value );
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Write( long Value )
        {
            try
            {
                DataWriter.Write( Value );
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Write( ulong Value )
        {
            try
            {
                DataWriter.Write( Value );
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Write( float Value )
        {
            try
            {
                DataWriter.Write( Value );
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Write( double Value )
        {
            try
            {
                DataWriter.Write( Value );
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Write( decimal Value )
        {
            try
            {
                DataWriter.Write( Value );
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Write( char Value )
        {
            try
            {
                DataWriter.Write( Value );
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool Write( char[] Value, int Length )
        {
            try
            {
                // TODO: 패킷 버퍼에 기록할 때 데이터 구조가 송수신측 모두 같은 구조를 갖는지 확인할 것
                //       예를 들어, 배열을 기록하기 전에 배열의 크기를 먼저 기록하고 내용을 기록하는 경우
                //       배열의 크기를 기록하지 않고 내용만 기록하는 경우
                DataWriter.Write( Value, 0, Length );
            }
            catch
            {
                return false;
            }

            return true;
        }
        #endregion
    }
}
