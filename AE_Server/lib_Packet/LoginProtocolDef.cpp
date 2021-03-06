#include "stdafx.h"
#include "..\lib_network\TPacket.h"
#include "./Protocol.h"
#include <list>
#include <string>
#include "LoginProtocolDef.h"


ServerInfo::ServerInfo()
{
	port = 0;
}
bool ServerInfo::Read(TSPacket* stream)
{
	// read serverIP
	unsigned short serverIPSize;
	if (!stream->readData(&serverIPSize, sizeof(unsigned short))) return false;
	void* serverIPData = stream->GetBuffer(serverIPSize);
	if (serverIPData != NULL)
		serverIP.assign((wchar_t*)serverIPData, serverIPSize);
	if (!stream->MoveHead(serverIPSize *sizeof(wchar_t))) return false;

	// read port
	if (!stream->readData(&port, sizeof(unsigned short))) return false;

	return true;
}

bool ServerInfo::Write(TSPacket* stream)
{
	// write serverIP
	unsigned short serverIPSize;
	serverIPSize = static_cast<unsigned short>(serverIP.size());
	if (!stream->writeData(&serverIPSize, sizeof(unsigned short))) return false;
	if (!stream->writeData((void*)serverIP.c_str(), sizeof(wchar_t) *serverIPSize)) return false;

	// write port
	if (!stream->writeData(&port, sizeof(unsigned short))) return false;

	return true;
}

bool ServerInfo::Read(TCPacket* stream)
{
	// read serverIP
	unsigned short serverIPSize;
	if (!stream->readData(&serverIPSize, sizeof(unsigned short))) return false;
	void* serverIPData = stream->GetBuffer(serverIPSize);
	if (serverIPData != NULL)
		serverIP.assign((wchar_t*)serverIPData, serverIPSize);
	if (!stream->MoveHead(serverIPSize *sizeof(wchar_t))) return false;

	// read port
	if (!stream->readData(&port, sizeof(unsigned short))) return false;

	return true;
}

bool ServerInfo::Write(TCPacket* stream)
{
	// write serverIP
	unsigned short serverIPSize;
	serverIPSize = static_cast<unsigned short>(serverIP.size());
	if (!stream->writeData(&serverIPSize, sizeof(unsigned short))) return false;
	if (!stream->writeData((void*)serverIP.c_str(), sizeof(wchar_t) *serverIPSize)) return false;

	// write port
	if (!stream->writeData(&port, sizeof(unsigned short))) return false;

	return true;
}

bool ServerInfo::operator==(const ServerInfo& opponent) const
{
	if (serverIP != opponent.serverIP) return false;

	if (port != opponent.port) return false;

	return true;
}

bool ServerInfo::operator!=(const ServerInfo& opponent) const
{
	return !(*this == opponent);
}

SnS_DISCONNECT_CMD::SnS_DISCONNECT_CMD()
{
	isAllServerShutdown = false;
}
bool SnS_DISCONNECT_CMD::Read(TSPacket* stream)
{
	// read base class
	if (!__super::Read(stream)) return false;

	// read isAllServerShutdown
	if (!stream->readData(&isAllServerShutdown, sizeof(bool))) return false;

	return true;
}

bool SnS_DISCONNECT_CMD::Write(TSPacket* stream)
{
	// write base class
	if (!__super::Write(stream)) return false;

	// write isAllServerShutdown
	if (!stream->writeData(&isAllServerShutdown, sizeof(bool))) return false;

	return true;
}

bool SnS_DISCONNECT_CMD::Read(TCPacket* stream)
{
	// read base class
	if (!__super::Read(stream)) return false;

	// read isAllServerShutdown
	if (!stream->readData(&isAllServerShutdown, sizeof(bool))) return false;

	return true;
}

bool SnS_DISCONNECT_CMD::Write(TCPacket* stream)
{
	// write base class
	if (!__super::Write(stream)) return false;

	// write isAllServerShutdown
	if (!stream->writeData(&isAllServerShutdown, sizeof(bool))) return false;

	return true;
}

bool SnS_DISCONNECT_CMD::operator==(const SnS_DISCONNECT_CMD& opponent) const
{
	if (isAllServerShutdown != opponent.isAllServerShutdown) return false;

	return true;
}

bool SnS_DISCONNECT_CMD::operator!=(const SnS_DISCONNECT_CMD& opponent) const
{
	return !(*this == opponent);
}

CnS_DISCONNECT_CMD::CnS_DISCONNECT_CMD()
{
}
bool CnS_DISCONNECT_CMD::Read(TSPacket* stream)
{
	// read base class
	if (!__super::Read(stream)) return false;

	return true;
}

bool CnS_DISCONNECT_CMD::Write(TSPacket* stream)
{
	// write base class
	if (!__super::Write(stream)) return false;

	return true;
}

bool CnS_DISCONNECT_CMD::Read(TCPacket* stream)
{
	// read base class
	if (!__super::Read(stream)) return false;

	return true;
}

bool CnS_DISCONNECT_CMD::Write(TCPacket* stream)
{
	// write base class
	if (!__super::Write(stream)) return false;

	return true;
}

bool CnS_DISCONNECT_CMD::operator==(const CnS_DISCONNECT_CMD& opponent) const
{
	return true;
}

bool CnS_DISCONNECT_CMD::operator!=(const CnS_DISCONNECT_CMD& opponent) const
{
	return !(*this == opponent);
}
