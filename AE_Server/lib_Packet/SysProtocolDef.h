#pragma once

#include "ProtoTypeDefine.h"


class SERVER_REGISTER_REQ : public Protocol
{
public:
	serverID_t hostServerID;
	U16 creationNo;

	// protocol tag
	static const int Tag = 1;
	virtual int GetTag() override { return 1; }

	// SERVER_REGISTER_REQ { hostServerID(serverID_t), creationNo(U16) }
	int GetVersion() { return 1240952989; }

	SERVER_REGISTER_REQ();
	virtual bool Read(TSPacket* stream);
	virtual bool Write(TSPacket* stream);
	virtual bool Read(TCPacket* stream);
	virtual bool Write(TCPacket* stream);
	bool operator==(const SERVER_REGISTER_REQ& opponent) const;
	bool operator!=(const SERVER_REGISTER_REQ& opponent) const;
};

class SERVER_REGISTER_ACK : public Protocol
{
public:
	unsigned int serverID;
	unsigned int protoVer;

	// protocol tag
	static const int Tag = 2;
	virtual int GetTag() override { return 2; }

	// SERVER_REGISTER_ACK { serverID(System.UInt32), protoVer(System.UInt32) }
	int GetVersion() { return -997676587; }

	SERVER_REGISTER_ACK();
	virtual bool Read(TSPacket* stream);
	virtual bool Write(TSPacket* stream);
	virtual bool Read(TCPacket* stream);
	virtual bool Write(TCPacket* stream);
	bool operator==(const SERVER_REGISTER_ACK& opponent) const;
	bool operator!=(const SERVER_REGISTER_ACK& opponent) const;
};

class ConnectionAddress
{
public:
	std::wstring IP;
	U16 port;

	// ConnectionAddress { IP(String), port(U16) }
	int GetVersion() { return -1302276090; }

	ConnectionAddress();
	virtual bool Read(TSPacket* stream);
	virtual bool Write(TSPacket* stream);
	virtual bool Read(TCPacket* stream);
	virtual bool Write(TCPacket* stream);
	bool operator==(const ConnectionAddress& opponent) const;
	bool operator!=(const ConnectionAddress& opponent) const;
};

class S2C_ONCLIENTCONNECT_CMD : public Protocol
{
public:
	objectHandle_t objHandle;

	// protocol tag
	static const int Tag = 5;
	virtual int GetTag() override { return 5; }

	// S2C_ONCLIENTCONNECT_CMD { objHandle(objectHandle_t) }
	int GetVersion() { return 1811771020; }

	S2C_ONCLIENTCONNECT_CMD();
	virtual bool Read(TSPacket* stream);
	virtual bool Write(TSPacket* stream);
	virtual bool Read(TCPacket* stream);
	virtual bool Write(TCPacket* stream);
	bool operator==(const S2C_ONCLIENTCONNECT_CMD& opponent) const;
	bool operator!=(const S2C_ONCLIENTCONNECT_CMD& opponent) const;
};
