#pragma once

#include "ProtoTypeDefine.h"


class ServerInfo
{
public:
	std::wstring serverIP;
	unsigned short port;

	// ServerInfo { serverIP(String), port(System.UInt16) }
	int GetVersion() { return 1931640989; }

	ServerInfo();
	virtual bool Read(TSPacket* stream);
	virtual bool Write(TSPacket* stream);
	virtual bool Read(TCPacket* stream);
	virtual bool Write(TCPacket* stream);
	bool operator==(const ServerInfo& opponent) const;
	bool operator!=(const ServerInfo& opponent) const;
};

class SnS_DISCONNECT_CMD : public Protocol
{
public:
	bool isAllServerShutdown;

	// protocol tag
	static const int Tag = 2167;
	virtual int GetTag() override { return 2167; }

	// SnS_DISCONNECT_CMD { isAllServerShutdown(System.Boolean) }
	int GetVersion() { return 1675708438; }

	SnS_DISCONNECT_CMD();
	virtual bool Read(TSPacket* stream);
	virtual bool Write(TSPacket* stream);
	virtual bool Read(TCPacket* stream);
	virtual bool Write(TCPacket* stream);
	bool operator==(const SnS_DISCONNECT_CMD& opponent) const;
	bool operator!=(const SnS_DISCONNECT_CMD& opponent) const;
};

class CnS_DISCONNECT_CMD : public Protocol
{
public:

	// protocol tag
	static const int Tag = 2168;
	virtual int GetTag() override { return 2168; }

	// CnS_DISCONNECT_CMD
	int GetVersion() { return -1432752071; }

	CnS_DISCONNECT_CMD();
	virtual bool Read(TSPacket* stream);
	virtual bool Write(TSPacket* stream);
	virtual bool Read(TCPacket* stream);
	virtual bool Write(TCPacket* stream);
	bool operator==(const CnS_DISCONNECT_CMD& opponent) const;
	bool operator!=(const CnS_DISCONNECT_CMD& opponent) const;
};
