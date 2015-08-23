#pragma once


class CPacketHandlerForServer
{
public:
	CPacketHandlerForServer( VOID ) {}
	~CPacketHandlerForServer( VOID ) {}

	int Func_SERVER_REGISTER_REQ( VOID* pSender, VOID* pRecdPack, VOID* pNonuse );
	int Func_W2S_GAMESERVER_ADR_CMD( VOID* pSender, VOID* pRecdPack, VOID* pNonuse );
	int Func_G2S_AFFILIATEDSERVERID_CMD( VOID* pSender, VOID* pRecdPack, VOID* pNonuse );
	int Func_W2S_LOGOUT_CMD( VOID* pSender, VOID* pRecdPack, VOID* pNonuse );
	int Func_SnS_DISCONNECT_CMD( VOID* pSender, VOID* pRecdPack, VOID* pNonuse );
	int Func_W2C_EMBEDING_CONNECT_ACK(VOID *pSender, VOID *pRecdPack, VOID *pNonuse);
};
