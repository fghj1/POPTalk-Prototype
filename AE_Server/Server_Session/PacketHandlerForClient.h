#pragma once


class CPacketHandlerForClient
{
public:
	CPacketHandlerForClient( VOID ) {}
	~CPacketHandlerForClient( VOID ) {}

	int Func_CnS_DISCONNECT_CMD( void* pSender, void* pRecdPack, void* pNonuse );
};
