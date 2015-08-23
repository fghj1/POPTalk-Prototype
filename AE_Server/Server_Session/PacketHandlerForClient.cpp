#include "StdAfx.h"
#include "../lib_Network/TPacket.h"
#include "./TClient.h"
#include "./SessionServer.h"
#include "./PacketHandlerForClient.h"


int CPacketHandlerForClient::Func_CnS_DISCONNECT_CMD( VOID* pSender, VOID* pRecdPack, VOID* pNonuse )
{
	TClient* pSenderSession = NULL;
	pSenderSession = static_cast<TClient*>( pSender );
	TSPacket* pCPacket = NULL;
	pCPacket = static_cast<TSPacket*>( pRecdPack );

	CnS_DISCONNECT_CMD protoCnSCMD;
	protoCnSCMD.Read( pCPacket );

	if( pSenderSession && pSenderSession->GetGOID() )
	{
		U64 uni64GOID = pSenderSession->GetGOID();

		( SER()->GetTcpClient() )->RemoveConnectedClient( uni64GOID );
		pSenderSession->SetIsConnecting( false );
		TLOG( LOG_DEBUG, _T( "Client(%I64d) connection terminated." ), uni64GOID );

		pSenderSession->ReinitAcpt( ( SER()->GetTcpClient() )->GetListenSocket() );
		TLOG( LOG_DEBUG, _T( "Connect count : %d" ), ( SER()->GetTcpClient() )->GetCCU() );
	}

	return 1;
}
