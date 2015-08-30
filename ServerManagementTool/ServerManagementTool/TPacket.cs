using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace ServerManagementTool
{
    public class TPacket : ObjectPool<TPacket>
    {
        public byte[] HeaderBuff = new byte[PACKETHEADERSIZE];
        public byte[] DataBuff = new byte[PACKETBUFFERSIZE];

        public Stream HeaderStream = null;
        public Stream DataStream = null;

        private const int PACKETHEADERSIZE = 12;
        private const int PACKETBUFFERSIZE = 8192 * 3;

        private BinaryWriter HeaderWriter;
        private BinaryReader HeaderReader;
        private BinaryWriter DataWriter;
        private BinaryReader DataReader;

        private TPacket()
        {
            HeaderStream = new MemoryStream( HeaderBuff );
            DataStream = new MemoryStream( DataBuff );

            HeaderWriter = new BinaryWriter( HeaderStream, System.Text.Encoding.Unicode );
            HeaderReader = new BinaryReader( HeaderStream, System.Text.Encoding.Unicode );
            DataWriter = new BinaryWriter( DataStream, System.Text.Encoding.Unicode );
            DataReader = new BinaryReader( DataStream, System.Text.Encoding.Unicode );
        }

        // TODO: 패킷 헤더 구현할 것
        // TODO: TPacket의 각 요소 값을 가져올 수 있는 편의성 함수 구현
        // TODO: 패킷 읽기, 쓰기를 여기에 구현한다.
        //       읽기와 쓰기는 데이터 타입별로 이뤄져야 한다.
        // TODO: 데이터 타입별 읽기, 쓰기를 할 수 있도록 부모 클래스로써 인터페이스를 둔다.
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

        public byte[] Data
        {
            get { return DataBuff; }
            set { DataBuff = value; }
        }
    }
}
