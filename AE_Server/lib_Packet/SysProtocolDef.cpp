#include "stdafx.h"
#include "..\lib_network\TPacket.h"
#include "./Protocol.h"
#include <list>
#include <string>
#include "SysProtocolDef.h"


SERVER_REGISTER_REQ::SERVER_REGISTER_REQ()
{
	hostServerID = 0;
	creationNo = 0;
}
bool SERVER_REGISTER_REQ::Read(TSPacket* stream)
{
	// read base class
	if (!__super::Read(stream)) return false;

	// read hostServerID
	if (!stream->readData(&hostServerID, sizeof(unsigned int))) return false;

	// read creationNo
	if (!stream->readData(&creationNo, sizeof(unsigned short))) return false;

	return true;
}

bool SERVER_REGISTER_REQ::Write(TSPacket* stream)
{
	// write base class
	if (!__super::Write(stream)) return false;

	// write hostServerID
	if (!stream->writeData(&hostServerID, sizeof(unsigned int))) return false;

	// write creationNo
	if (!stream->writeData(&creationNo, sizeof(unsigned short))) return false;

	return true;
}

bool SERVER_REGISTER_REQ::Read(TCPacket* stream)
{
	// read base class
	if (!__super::Read(stream)) return false;

	// read hostServerID
	if (!stream->readData(&hostServerID, sizeof(unsigned int))) return false;

	// read creationNo
	if (!stream->readData(&creationNo, sizeof(unsigned short))) return false;

	return true;
}

bool SERVER_REGISTER_REQ::Write(TCPacket* stream)
{
	// write base class
	if (!__super::Write(stream)) return false;

	// write hostServerID
	if (!stream->writeData(&hostServerID, sizeof(unsigned int))) return false;

	// write creationNo
	if (!stream->writeData(&creationNo, sizeof(unsigned short))) return false;

	return true;
}

bool SERVER_REGISTER_REQ::operator==(const SERVER_REGISTER_REQ& opponent) const
{
	if (hostServerID != opponent.hostServerID) return false;

	if (creationNo != opponent.creationNo) return false;

	return true;
}

bool SERVER_REGISTER_REQ::operator!=(const SERVER_REGISTER_REQ& opponent) const
{
	return !(*this == opponent);
}

SERVER_REGISTER_ACK::SERVER_REGISTER_ACK()
{
	serverID = 0;
	protoVer = 0;
}
bool SERVER_REGISTER_ACK::Read(TSPacket* stream)
{
	// read base class
	if (!__super::Read(stream)) return false;

	// read serverID
	if (!stream->readData(&serverID, sizeof(unsigned int))) return false;

	// read protoVer
	if (!stream->readData(&protoVer, sizeof(unsigned int))) return false;

	return true;
}

bool SERVER_REGISTER_ACK::Write(TSPacket* stream)
{
	// write base class
	if (!__super::Write(stream)) return false;

	// write serverID
	if (!stream->writeData(&serverID, sizeof(unsigned int))) return false;

	// write protoVer
	if (!stream->writeData(&protoVer, sizeof(unsigned int))) return false;

	return true;
}

bool SERVER_REGISTER_ACK::Read(TCPacket* stream)
{
	// read base class
	if (!__super::Read(stream)) return false;

	// read serverID
	if (!stream->readData(&serverID, sizeof(unsigned int))) return false;

	// read protoVer
	if (!stream->readData(&protoVer, sizeof(unsigned int))) return false;

	return true;
}

bool SERVER_REGISTER_ACK::Write(TCPacket* stream)
{
	// write base class
	if (!__super::Write(stream)) return false;

	// write serverID
	if (!stream->writeData(&serverID, sizeof(unsigned int))) return false;

	// write protoVer
	if (!stream->writeData(&protoVer, sizeof(unsigned int))) return false;

	return true;
}

bool SERVER_REGISTER_ACK::operator==(const SERVER_REGISTER_ACK& opponent) const
{
	if (serverID != opponent.serverID) return false;

	if (protoVer != opponent.protoVer) return false;

	return true;
}

bool SERVER_REGISTER_ACK::operator!=(const SERVER_REGISTER_ACK& opponent) const
{
	return !(*this == opponent);
}

ConnectionAddress::ConnectionAddress()
{
	port = 0;
}
bool ConnectionAddress::Read(TSPacket* stream)
{
	// read IP
	unsigned short IPSize;
	if (!stream->readData(&IPSize, sizeof(unsigned short))) return false;
	void* IPData = stream->GetBuffer(IPSize);
	if (IPData != NULL)
		IP.assign((wchar_t*)IPData, IPSize);
	if (!stream->MoveHead(IPSize *sizeof(wchar_t))) return false;

	// read port
	if (!stream->readData(&port, sizeof(unsigned short))) return false;

	return true;
}

bool ConnectionAddress::Write(TSPacket* stream)
{
	// write IP
	unsigned short IPSize;
	IPSize = static_cast<unsigned short>(IP.size());
	if (!stream->writeData(&IPSize, sizeof(unsigned short))) return false;
	if (!stream->writeData((void*)IP.c_str(), sizeof(wchar_t) *IPSize)) return false;

	// write port
	if (!stream->writeData(&port, sizeof(unsigned short))) return false;

	return true;
}

bool ConnectionAddress::Read(TCPacket* stream)
{
	// read IP
	unsigned short IPSize;
	if (!stream->readData(&IPSize, sizeof(unsigned short))) return false;
	void* IPData = stream->GetBuffer(IPSize);
	if (IPData != NULL)
		IP.assign((wchar_t*)IPData, IPSize);
	if (!stream->MoveHead(IPSize *sizeof(wchar_t))) return false;

	// read port
	if (!stream->readData(&port, sizeof(unsigned short))) return false;

	return true;
}

bool ConnectionAddress::Write(TCPacket* stream)
{
	// write IP
	unsigned short IPSize;
	IPSize = static_cast<unsigned short>(IP.size());
	if (!stream->writeData(&IPSize, sizeof(unsigned short))) return false;
	if (!stream->writeData((void*)IP.c_str(), sizeof(wchar_t) *IPSize)) return false;

	// write port
	if (!stream->writeData(&port, sizeof(unsigned short))) return false;

	return true;
}

bool ConnectionAddress::operator==(const ConnectionAddress& opponent) const
{
	if (IP != opponent.IP) return false;

	if (port != opponent.port) return false;

	return true;
}

bool ConnectionAddress::operator!=(const ConnectionAddress& opponent) const
{
	return !(*this == opponent);
}

S2C_ONCLIENTCONNECT_CMD::S2C_ONCLIENTCONNECT_CMD()
{
	objHandle = 0;
}
bool S2C_ONCLIENTCONNECT_CMD::Read(TSPacket* stream)
{
	// read base class
	if (!__super::Read(stream)) return false;

	// read objHandle
	if (!stream->readData(&objHandle, sizeof(unsigned __int64))) return false;

	return true;
}

bool S2C_ONCLIENTCONNECT_CMD::Write(TSPacket* stream)
{
	// write base class
	if (!__super::Write(stream)) return false;

	// write objHandle
	if (!stream->writeData(&objHandle, sizeof(unsigned __int64))) return false;

	return true;
}

bool S2C_ONCLIENTCONNECT_CMD::Read(TCPacket* stream)
{
	// read base class
	if (!__super::Read(stream)) return false;

	// read objHandle
	if (!stream->readData(&objHandle, sizeof(unsigned __int64))) return false;

	return true;
}

bool S2C_ONCLIENTCONNECT_CMD::Write(TCPacket* stream)
{
	// write base class
	if (!__super::Write(stream)) return false;

	// write objHandle
	if (!stream->writeData(&objHandle, sizeof(unsigned __int64))) return false;

	return true;
}

bool S2C_ONCLIENTCONNECT_CMD::operator==(const S2C_ONCLIENTCONNECT_CMD& opponent) const
{
	if (objHandle != opponent.objHandle) return false;

	return true;
}

bool S2C_ONCLIENTCONNECT_CMD::operator!=(const S2C_ONCLIENTCONNECT_CMD& opponent) const
{
	return !(*this == opponent);
}
