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
    }
}
