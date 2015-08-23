using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace ServerManagementTool
{
    public class CircleBuffer<T>
    {
        private int ReservedSize;
        private int UsedSize;
        private int Head;
        private int Tail;
        private T[] DataField;

        public CircleBuffer( int DefaultCapacity )
        {
            if( ReservedSize < 0 )
                return;

            ReservedSize = DefaultCapacity;
            UsedSize = 0;
            Head = 0;
            Tail = 0;
            DataField = new T[ReservedSize];
        }

        public int Capacity
        {
            get { return ReservedSize; }
            set
            {
                if( value <= ReservedSize )
                    return;

                T[] NewDataField = new T[value];
                if( 0 < UsedSize )
                    CopyTo( 0, NewDataField, 0, UsedSize );
                DataField = NewDataField;
                ReservedSize = value;
            }
        }

        public int Size
        {
            get { return UsedSize; }
        }

        public void CopyTo( int SourcePos, T[] TargetArray, int TargetPos, int CopiableSize )
        {
            if( UsedSize < CopiableSize )
            {
                // TODO: 처리 대상이 되는 데이터보다 더 많은 데이터를 추출하려고 한다.
                //       C#의 올바른 예외 처리 방식을 이해하고 적절한 처리를 추가해야 한다.
            }

            int CurtPos = 0;
            for( CurtPos = Head; CurtPos < CopiableSize; ++CurtPos )
            {
                if( ReservedSize == CurtPos )  // NOTE: 순환 목적에 맞게 위치 재조정
                    CurtPos = 0;

                TargetArray[TargetPos++] = DataField[CurtPos];
            }
        }

        public bool Seek( long offset )
        {
            if( offset > UsedSize )
                return false;

            int MoveHead = Head + ( int )offset;

            if( ReservedSize <= MoveHead )
                MoveHead -= ReservedSize;

            if( Tail < MoveHead )
                return false;

            Head = MoveHead;
            UsedSize -= ( int )offset;

            return true;
        }
    }

    public class CircleStream : Stream
    {
        private CircleBuffer<byte> Buffer;

        public CircleStream( int Capacity )
        {
            Buffer = new CircleBuffer<byte>( Capacity );
        }

        public override long Seek( long offset, SeekOrigin origin )
        {
            if( origin == SeekOrigin.Current )  // TODO: origin != SeekOrigin.Current 으로 해서 else를 통해 어떤 일이 벌어지는지 테스트할 것
            {
                if( Buffer.Seek( offset ) )
                    return offset;
                else
                    return 0;
            }
            else
                throw new NotImplementedException();
        }

        public override long Length
        {
            get { return Buffer.Size; }
        }

        public int PeekBuffer( byte[] TempBuff )
        {
            int CopiableSize = Buffer.Size;
            if( CopiableSize > TempBuff.Length )
                CopiableSize = TempBuff.Length;

            Buffer.CopyTo( 0, TempBuff, 0, CopiableSize );

            return CopiableSize;
        }

        public bool PeekPacketSize( out ushort PacketSize )
        {
            if( Length < sizeof( ushort ) )
            {
                PacketSize = 0;
                return false;
            }

            byte[] TempPacketSize = new byte[sizeof( ushort )];

            Buffer.CopyTo( 0, TempPacketSize, 0, sizeof( ushort ) );
            PacketSize = BitConverter.ToUInt16( TempPacketSize, 0 );  // TODO: C#과 C++의 byte 배열이 달라서 이렇게 처리하는 것일까?

            return true;
        }
    }
}
