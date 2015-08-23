#include "stdafx.h"
#include "./PacketHandlerMGR.h"

#define REGISTER_CLIENT_DELEGATE(proto)	\
	AddHandler(proto::Tag, MakeDelegate(&m_handlerDefForClient, &CPacketHandlerForClient::Func_##proto));

#define REGISTER_SERVER_DELEGATE(proto)	\
	AddHandler(proto::Tag, MakeDelegate(&m_handlerDefForServer, &CPacketHandlerForServer::Func_##proto));


CPacketHandlerMGR::CPacketHandlerMGR( VOID ):m_pmapHandlers( NULL )
{
	//m_RegdProtocol.ClearAllNumber();
}

bool CPacketHandlerMGR::InitPacketHandlerMGR( VOID )
{
	//m_RegdProtocol.ClearAllNumber();
	m_pmapHandlers = new TMap<DWORD, tPacketHandler, NumberHash<DWORD>>;
	if( m_pmapHandlers == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to create the object as TMap type." ) );
		return false;
	}

	REGISTER_SERVER_DELEGATE(SnS_DISCONNECT_CMD)
	REGISTER_CLIENT_DELEGATE(CnS_DISCONNECT_CMD)

	return true;
}

VOID CPacketHandlerMGR::ReleasePacketHandlerMGR( VOID )
{
	SAFE_DELETE( m_pmapHandlers );
}

/*
bool CPacketHandlerMGR::IsRegdProtocol( U16 nProtocol )
{
	return m_RegdProtocol.IsSet( nProtocol );
}
*/

tPacketHandler CPacketHandlerMGR::GetHandler( U16 nProtocol )
{
	tPacketHandler handler;

	m_pmapHandlers->Find( nProtocol, &handler );
	//if( m_pmapHandlers->Find( nProtocol, &handler ) == false )
		//TLOG( LOG_DEBUG, _T( "[ERROR] Failed to find a packet protocol handler related protocol(%d)." ), nProtocol );

	return handler;
}

eServerType CPacketHandlerMGR::GetDestination( U16 nProtocol )
{
	CAtlMap<int, eServerType>::CPair* pElement = NULL;

	pElement = m_mapProtocolToAffiliatedServer.Lookup( nProtocol );
	if( pElement == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[WARNING] Because there is no matching key, failed to extract destination from the protocol destination map." ) );
		return SERVER_NONE;
	}

	return pElement->m_value;
}

VOID CPacketHandlerMGR::AddHandler( U16 nProtocol, tPacketHandler& rHandler )
{
	/*
	if( m_RegdProtocol.AddNumber( nProtocol ) == false )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to add a specified value(%d)." ), nProtocol );
		return;
	}
	*/

	if( m_pmapHandlers->Add( nProtocol, rHandler ) == false )
		TLOG( LOG_DEBUG, _T( "[WARNING] A protocol(%d) has already been registered." ), nProtocol );
}
