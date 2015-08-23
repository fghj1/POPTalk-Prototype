#include "StdAfx.h"
#include "../lib_Network/TPacket.h"
#include "./TSession.h"
#include "./SessionServer.h"
#include "./PacketHandlerForServer.h"


int CPacketHandlerForServer::Func_SnS_DISCONNECT_CMD( VOID* pSender, VOID* pRecdPack, VOID* pNonuse )
{
	TSession* pSenderSession = static_cast<TSession*>( pSender );
	TSPacket* pPacket = static_cast<TSPacket*>( pRecdPack );

	ServerID idSenderSession;
	idSenderSession.value = pSenderSession->GetserverID();

	SnS_DISCONNECT_CMD proto;
	proto.Read( pPacket );

	if( pSenderSession )
		pSenderSession->OnIODisconnected();

	return 1;
}
