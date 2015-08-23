#include "StdAfx.h"
#include "./SessionServer.h"
#include "./ClientTcp.h"


CClientTcp::CClientTcp( VOID ):m_pListen( NULL ), m_bIsRebirthListen( false ),
							   m_pInactiveTServerElem( NULL ), m_pActiveTServerElem( NULL ), m_pmapConnectedServerList( NULL ),
							   m_pInactiveTClientElem( NULL ), m_pActiveTClientElem( NULL ), m_pmapConnectedClientList( NULL ),
							   m_pWorldServer( NULL )
{
	m_packetHandlerMGR.InitPacketHandlerMGR();

	m_pInactiveTServerElem = new TFixedMemoryPool<TSession, MAX_PEER_IN_PROTOTYPESERVER>;
	if( m_pInactiveTServerElem == NULL )
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to create the objects as TFixedMemoryPool type." ) );

	m_pActiveTServerElem = new TMap<ULONG_PTR, TSession*, NumberHash<ULONG_PTR>>;
	if( m_pActiveTServerElem == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to create the object as TMap type." ) );
		SAFE_DELETE( m_pInactiveTServerElem );
	}

	m_pmapConnectedServerList = new TMap<serverID_t, TSession*, NumberHash<serverID_t>>;
	if( m_pmapConnectedServerList == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to create the object as TMap type." ) );
		SAFE_DELETE( m_pInactiveTServerElem );
		SAFE_DELETE( m_pActiveTServerElem );
	}
}

bool CClientTcp::Begin( VOID )
{
	if( CIocpForNetwork::Begin() == false )
		return false;

	m_pListen = new CNetwork( NETWORK_KIND_LISTEN_CLIENT );
	if( m_pListen == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to create the object as CNetwork type." ) );
		return false;
	}

	if( m_pListen->StartN() == FALSE )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to initialize the object as CNetwork type." ) );
		return false;
	}

	if( m_pListen->CreateNBOLSocket() == FALSE )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to create the socket." ) );
		return false;
	}

	if( m_pListen->Listen( SESSION_PORT_FOR_CLIENT, MAX_USER_IN_SESSIONSERVER ) == FALSE )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to place a socket in a state in which it is listening for an incoming connection." ) );
		return false;
	}

	if( RegisterSocketToIocp( m_pListen->GetNSocket(), reinterpret_cast<ULONG_PTR>( m_pListen ) ) == false )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to associate the IOCP with a specified socket handle." ) );
		return false;
	}

	if( CreateClientConnection( m_pListen->GetNSocket() ) == false )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to create the awaiting connection object." ) );
		return false;
	}

	return true;
}

VOID CClientTcp::End( VOID )
{
	CloseWorkThreadAndIOCP();

	EndpListen();

	ReleaseClientConnection();
	ReleaseServerConnection();

	m_packetHandlerMGR.ReleasePacketHandlerMGR();
}

SOCKET CClientTcp::GetListenSocket( VOID )
{
	SCOPED_LOCK( m_CS )
	{
		if( m_pListen )
		{
			return m_pListen->GetNSocket();
		}
	}
	
	return NULL;
}

bool CClientTcp::EndpListen( VOID )
{
	SCOPED_LOCK( m_CS )
	{
		if( m_pListen )
		{
			if( m_pListen->EndN() == FALSE )
				return false;
			else
				SAFE_DELETE( m_pListen );
		}
	}

	return true;
}

bool CClientTcp::RebirthOfListenSocket( VOID )
{
	if( m_bIsRebirthListen == false )
		return true;

	m_bIsRebirthListen = false;

	if( BreakListenSocket() == false )
		return false;

	m_pListen = new CNetwork( NETWORK_KIND_LISTEN_CLIENT );
	if( m_pListen == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to create the object as CNetwork type." ) );
		return false;
	}

	if( m_pListen->StartN() == FALSE )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to initialize the object as CNetwork type." ) );
		return false;
	}

	if( m_pListen->CreateNBOLSocket() == FALSE )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to create the socket." ) );
		return false;
	}

	if( m_pListen->Listen( SESSION_PORT_FOR_CLIENT, MAX_USER_IN_SESSIONSERVER ) == FALSE )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to place a socket in a state in which it is listening for an incoming connection." ) );
		return false;
	}

	if( RegisterSocketToIocp( m_pListen->GetNSocket(), reinterpret_cast<ULONG_PTR>( m_pListen ) ) == false )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to associate the IOCP with a specified socket handle." ) );
		return false;
	}

	U16 index = 0, creationNo = 0;
	TClient* pTClient = NULL;
	size_t uniRemainderObject = m_pInactiveTClientElem->GetRemainder();
	for( index = 0; index < uniRemainderObject; ++index )
	{
		pTClient = GetTClientFromPool();
		if( pTClient == NULL )
		{
			ReleaseClientConnection();
			return false;
		}

		creationNo = pTClient->GetcreationNo();
		// TODO: creationNo과 이미 연결 중인 객체(m_pmapConnectedClientList)가 가진 m_creationNo이 중복되지 않는지 검증이 필요하다.

		if( ( pTClient->InitAcpt( this, creationNo, ( m_pListen->GetNSocket() ) ) ) == false )
		{
			TLOG( LOG_DEBUG, _T( "[ERROR] Failed to initialize the raw object to accepted state." ) );
			ReleaseClientConnection();
			return false;
		}

		pTClient = NULL;
	}

	return true;
}

bool CClientTcp::CreateServerConnection( NetAddress& rConnectionAddr )
{
	LPSTR lpszAddr = rConnectionAddr.ToString();  // The size of buffer in NetAddress type : 20

	if( strnlen( lpszAddr, 20 ) >= 20 || rConnectionAddr.port <= 0 )
		return false;

	TSession* pOtherServer = GetTServerFromPool();
	if( pOtherServer == NULL )
	{
		ReleaseServerConnection();
		return false;
	}

	if( pOtherServer->StartP() == FALSE )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to initialize the object as CPeer type." ) );
		ReturnTServerToPool( pOtherServer );
		return false;
	}

	if( pOtherServer->CreateNBOLSocket() == FALSE )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to create the socket." ) );
		pOtherServer->EndP();
		ReturnTServerToPool( pOtherServer );
		return false;
	}

	if( sizeof( TCHAR ) == sizeof( WCHAR ) )
	{
		WCHAR szAddress[20] = {0,};

		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, lpszAddr, -1, szAddress, 20 );
		TLOG( LOG_CRT, _T( "Connecting to the %s:%d..." ), szAddress, rConnectionAddr.port );
	}
	else
		TLOG( LOG_CRT, _T( "Connecting to the %s:%d..." ), lpszAddr, rConnectionAddr.port );

	if( pOtherServer->CreateConnection( lpszAddr, rConnectionAddr.port ) == false )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to connect the opposite side." ) );
		pOtherServer->EndP();
		ReturnTServerToPool( pOtherServer );
		return false;
	}

	pOtherServer->SetClientTcp( this );
	if( pOtherServer->OnIOConnected() == false )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Immediately after connection, failed to process the next." ) );
		pOtherServer->EndP();
		ReturnTServerToPool( pOtherServer );
		return false;
	}

	return true;
}

VOID CClientTcp::DisconnectedServer( TSession* pServer )
{
	if( pServer == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[WARNING] Parameter(%#p) is invalid." ), pServer );
		return;
	}

	ServerID tempServerID;
	tempServerID.value = pServer->GetserverID();
	m_pmapConnectedServerList->Remove( tempServerID.value );
}

VOID CClientTcp::ReusableSession( TSession* pReusableSession )
{
	if( pReusableSession == NULL )
		return;

	itorTSessionList itorStart, itorEnd, itor;
	TSession* pElement = NULL;

	SCOPED_LOCK( m_CS )
	{
		itorStart = m_listUsableTServerElem.begin();
		itorEnd = m_listUsableTServerElem.end();
		for( itor = itorStart; itor != itorEnd; )
		{
			pElement = *itor;
			if( pElement == NULL )
			{
				itor = m_listUsableTServerElem.erase( itor );
				continue;
			}

			if( pReusableSession == pElement )
				return;

			++itor;
		}

		m_listUsableTServerElem.push_back( pReusableSession );

		// NOTE: m_pInactiveTServerElem을 사용하지 않고 m_listUsableTServerElem을 사용하는 이유
		//		 m_pInactiveTServerElem은 main() 함수를 처리하는 스레드에서만 사용하고 있습니다.
		//		 본 함수는 특정 스레드에 의해서 처리되는 것이 아닌 여러 스레드에 의해 처리됩니다.
		//		 따라서 m_listUsableTServerElem이 아닌 m_pInactiveTServerElem을 사용하게 되면
		//		 m_pInactiveTServerElem을 사용하는 모든 곳에 다중 스레드에 안전하도록 수정해야
		//		 하는데 이 비용보다 m_listUsableTServerElem을 사용하는 비용이 더 적기 때문에
		//		 m_listUsableTServerElem을 사용했습니다.
	}
}

VOID CClientTcp::FindConnectedClient( objectHandle_t GOID, TClient** ppClient )
{
	m_pmapConnectedClientList->Find( GOID, ppClient );
}

bool CClientTcp::SendToServer( serverID_t toDestServerID, TSPacket& rspkPack )
{
	if( toDestServerID == 0 )
	{
		TLOG( LOG_DEBUG, _T( "[WARNING] The first parameter(%d) is invalid." ), toDestServerID );
		return false;
	}

	ServerID serverID;
	TSession* pConnectedServer = NULL;
	std::vector<TSession*> vectDestServerList;

	serverID.value = toDestServerID;
	if( serverID.items.m_startupNo == 0 )
	{
		if( m_pmapConnectedServerList->Count() > 0 )
		{
			MapIterator<serverID_t, TSession*, NumberHash<serverID_t>> itor( *m_pmapConnectedServerList );
			ServerID tempServerID;
			for( itor.Begin(); itor.IsContinuous(); )
			{
				itor.CurrentItem( &tempServerID.value, &pConnectedServer );
				if( pConnectedServer == NULL )
				{
					itor.Next();
					m_pmapConnectedServerList->Remove( tempServerID.value );
					continue;
				}

				itor.Next();
				if( tempServerID.items.m_classification == serverID.items.m_classification )
					vectDestServerList.push_back( pConnectedServer );
			}
		}
	}
	else
	{
		m_pmapConnectedServerList->Find( toDestServerID, &pConnectedServer );
		if( pConnectedServer == NULL )
			m_pmapConnectedServerList->Remove( toDestServerID );
		else
			vectDestServerList.push_back( pConnectedServer );
	}

	if( vectDestServerList.empty() )
	{
		TLOG( LOG_DEBUG,
			  _T( "[WARNING] Because a server object that matches a specified server ID(%d) was not found, failed to send the packet." ),
			  toDestServerID );
		return false;
	}

	std::vector<TSession*>::iterator itor, itorStart, itorEnd;
	itorStart = vectDestServerList.begin();
	itorEnd = vectDestServerList.end();
	for( itor = itorStart; itor != itorEnd; ++itor )
	{
		pConnectedServer = *itor;
		if( pConnectedServer && pConnectedServer->GetIsConnecting() == false )
			continue;

		// TODO: 서버간 재연결 작업
		//		 처리가 실패하였을 경우, 연결을 끊었다가 다시 연결하도록 처리해야 한다. IOCP에서는 ReinitAcpt()가 이런 역할을 한다.
		if( pConnectedServer && pConnectedServer->WritePacket( rspkPack.Length(), rspkPack.getBuffer() ) < 0 )
		{
			TLOG( LOG_DEBUG,
				  _T( "[ERROR] Failed to send packet to server(%d). Disconnecting with the server(%d)." ),
				  toDestServerID, toDestServerID );
			pConnectedServer->Disconnect();
			return false;
		}
	}
	vectDestServerList.clear();

	return true;
}

VOID CClientTcp::ServerSessShutdown( bool bIsWholeServerSess/* = true*/ )
{
	if( m_pmapConnectedServerList == NULL )
		return;

	if( m_pmapConnectedServerList->Count() <= 0 )
		return;

	serverID_t serverIDValue = 0;
	TSession* pOtherServer = NULL;

	MapIterator<serverID_t, TSession*, NumberHash<serverID_t>> itor( *m_pmapConnectedServerList );
	for( itor.Begin(); itor.IsContinuous(); )
	{
		itor.CurrentItem( &serverIDValue, &pOtherServer );
		if( pOtherServer == NULL )
		{
			itor.Next();
			m_pmapConnectedServerList->Remove( serverIDValue );
			continue;
		}

		itor.Next();
		if( bIsWholeServerSess )
			pOtherServer->InduceSafeDisconnection();
		else
		{
			if( pOtherServer->GetWaitingToDisconnect() )
			{
				pOtherServer->InduceSafeDisconnection();
				pOtherServer->SetWaitingToDisconnect( false );
			}
		}
	}
}

VOID CClientTcp::ClientSessShutdown( VOID )
{
	if( m_pmapConnectedClientList != NULL )
	{
		if( m_pmapConnectedClientList->Count() > 0 )
		{
			objectHandle_t GOID = 0;
			TClient* pTClient = NULL;

			MapIterator<objectHandle_t, TClient*, NumberHash<objectHandle_t>> itor( *m_pmapConnectedClientList );
			for( itor.Begin(); itor.IsContinuous(); )
			{
				itor.CurrentItem( &GOID, &pTClient );
				if( pTClient == NULL )
				{
					itor.Next();
					m_pmapConnectedClientList->Remove( GOID );
					continue;
				}

				itor.Next();
				pTClient->InduceSafeDisconnection();
			}
		}
	}
}

VOID CClientTcp::UpdateCCU( VOID )
{
	LONG lCCU = static_cast<LONG>( GetCCU() );
	InterlockedExchange( &g_CCU, lCCU );
}

VOID CClientTcp::DisplayConnectedServerNetworkInfo( VOID )
{
	if( m_pmapConnectedServerList == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[WARNING] Do not create the object as TMap type." ) );
		return;
	}

	MapIterator<serverID_t, TSession*, NumberHash<serverID_t>> itor( *m_pmapConnectedServerList );
	ServerID tempServerID;
	TSession* pConnectedServer = NULL;
	SOCKADDR_IN ConnectedServerSocketInfo;
	INT nSize = sizeof( SOCKADDR_IN );
	serverID_t serverIDValue = 0;
	std::tstring serverIP;
	u_short serverPort = 0;

	_tprintf_s( _T( "\nConnected server network address information\n" ) );
	for( itor.Begin(); itor.IsContinuous(); )
	{
		itor.CurrentItem( &tempServerID.value, &pConnectedServer );
		if( pConnectedServer == NULL )
		{
			itor.Next();
			m_pmapConnectedServerList->Remove( tempServerID.value );
			continue;
		}

		itor.Next();
		if( getpeername( pConnectedServer->GetNSocket(), ( SOCKADDR* )&ConnectedServerSocketInfo, &nSize ) == SOCKET_ERROR )
		{
			TLOG( LOG_DEBUG, _T( "[ERROR] ErrNo.%d. Failed to get the socket info." ), WSAGetLastError() );
			continue;
		}
		serverIDValue = pConnectedServer->GetserverID();
		serverIP = M2W( inet_ntoa( ConnectedServerSocketInfo.sin_addr ) );
		serverPort = ntohs( ConnectedServerSocketInfo.sin_port );
		_tprintf_s( _T( "Remote info. %u - IP : %s, Port : %u\n" ), serverIDValue, serverIP.c_str(), serverPort );

		if( getsockname( pConnectedServer->GetNSocket(), ( SOCKADDR* )&ConnectedServerSocketInfo, &nSize ) == SOCKET_ERROR )
		{
			TLOG( LOG_DEBUG, _T( "[ERROR] ErrNo.%d. Failed to get the socket info." ), WSAGetLastError() );
			continue;
		}
		serverIP = M2W( inet_ntoa( ConnectedServerSocketInfo.sin_addr ) );
		serverPort = ntohs( ConnectedServerSocketInfo.sin_port );
		_tprintf_s( _T( "Local info. %u - IP : %s, Port : %u\n" ), serverIDValue, serverIP.c_str(), serverPort );
	}
}

VOID CClientTcp::OnIOConnected( VOID* pObject )
{
	if( pObject == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[WARNING] Parameter(%#p) is invalid." ), pObject );
		return;
	}

	CNetwork* pAbstractObj = reinterpret_cast<CNetwork*>( pObject );
	INT iNetworkKind = pAbstractObj->GetNetworkKind();
	switch( iNetworkKind )
	{
	case NETWORK_KIND_SERVER:
		{
			TSession* pConnectedServer = reinterpret_cast<TSession*>( pObject );

			if( pConnectedServer->OnIOConnected() == false )
				return;
		}
		break;

	case NETWORK_KIND_CLIENT:
		{
			TClient* pConnectedClient = reinterpret_cast<TClient*>( pObject );

			if( pConnectedClient->OnIOConnected() == false )
				return;

			TLOG( LOG_DEBUG, _T( "Connect count : %d" ), GetCCU() );
		}
		break;

	default:
		TLOG( LOG_DEBUG, _T( "[WARNING] The parameter(%d) of switch() is invalid." ), iNetworkKind );
		return;
	}
}

VOID CClientTcp::OnIODisconnected( VOID* pObject, bool bIsForcedDisconnection/* = false*/ )
{
	if( pObject == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[WARNING] Parameter(%#p) is invalid." ), pObject );
		return;
	}

	CNetwork* pAbstractObj = reinterpret_cast<CNetwork*>( pObject );
	INT iNetworkKind = pAbstractObj->GetNetworkKind();
	switch( iNetworkKind )
	{
	case NETWORK_KIND_SERVER:
		{
			TSession* pConnectedServer = reinterpret_cast<TSession*>( pObject );

			if( bIsForcedDisconnection )
				pConnectedServer->Disconnect();
			else
				pConnectedServer->InduceSafeDisconnection();
		}
		break;

	case NETWORK_KIND_CLIENT:
		{
			TClient* pConnectedClient = reinterpret_cast<TClient*>( pObject );

			pConnectedClient->OnIODisconnected();
		}
		break;

	case NETWORK_KIND_LISTEN_CLIENT:
		TLOG( LOG_DEBUG, _T( "[ERROR] Problem associated with the listening socket occurred in the process of IOCP." ) );
		m_bIsRebirthListen = true;
		break;

	default:
		TLOG( LOG_DEBUG, _T( "[WARNING] The parameter(%d) of switch() is invalid." ), iNetworkKind );
		break;
	}
}

VOID CClientTcp::OnIORead( VOID* pObject, U32 dwDataLength )
{
	if( pObject == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[WARNING] Parameter(%#p) is invalid." ), pObject );
		return;
	}

	CNetwork* pAbstractObj = reinterpret_cast<CNetwork*>( pObject );
	INT iNetworkKind = pAbstractObj->GetNetworkKind();
	switch( iNetworkKind )
	{
	case NETWORK_KIND_SERVER:
		{
			TSession* pConnectedServer = reinterpret_cast<TSession*>( pObject );

			pConnectedServer->OnIORead( dwDataLength );
		}
		break;

	case NETWORK_KIND_CLIENT:
		{
			TClient* pConnectedClient = reinterpret_cast<TClient*>( pObject );

			pConnectedClient->OnIORead( dwDataLength );
		}
		break;

	default:
		TLOG( LOG_DEBUG, _T( "[WARNING] The parameter(%d) of switch() is invalid." ), iNetworkKind );
		break;
	}
}

VOID CClientTcp::OnIOWrote( VOID* pObject )
{
	if( pObject == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[WARNING] Parameter(%#p) is invalid." ), pObject );
		return;
	}

	CNetwork* pAbstractObj = reinterpret_cast<CNetwork*>( pObject );
	INT iNetworkKind = pAbstractObj->GetNetworkKind();
	switch( iNetworkKind )
	{
	case NETWORK_KIND_SERVER:
		{
			TSession* pConnectedServer = reinterpret_cast<TSession*>( pObject );

			pConnectedServer->OnIOWrote();
		}
		break;

	case NETWORK_KIND_CLIENT:
		{
			TClient* pConnectedClient = reinterpret_cast<TClient*>( pObject );

			pConnectedClient->OnIOWrote();
		}
		break;

	default:
		TLOG( LOG_DEBUG, _T( "[WARNING] The parameter(%d) of switch() is invalid." ), iNetworkKind );
		break;
	}
}

VOID CClientTcp::StartShutdown( VOID )
{
	SER()->SetbFinishServer( TRUE );
}

TSession* CClientTcp::GetTServerFromPool( VOID )
{
	TSession* pOtherServer = NULL;

	SCOPED_LOCK( m_CS )
	{
		while( m_listUsableTServerElem.empty() == false )
		{
			pOtherServer = m_listUsableTServerElem.front();
			m_listUsableTServerElem.pop_front();

			if( pOtherServer != NULL )
				return pOtherServer;
		}
	}

	pOtherServer = m_pInactiveTServerElem->AllocateElement();
	if( pOtherServer == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to extract a raw object of TSession type." ) );
		return NULL;
	}
	m_pActiveTServerElem->Add( ( ULONG_PTR )pOtherServer, pOtherServer );

	return pOtherServer;
}

bool CClientTcp::ReturnTServerToPool( TSession* pTServer )
{
	m_pInactiveTServerElem->ReturnElement( pTServer );
	return m_pActiveTServerElem->Remove( ( ULONG_PTR )pTServer );
}

VOID CClientTcp::ReleaseServerConnection( VOID )
{
	if( m_pInactiveTServerElem == NULL || m_pActiveTServerElem == NULL || m_pmapConnectedServerList == NULL )
		return;

	TSession* pOtherServer = NULL;

	if( m_pmapConnectedServerList->Count() > 0 )
	{
		serverID_t serverID = 0;

		MapIterator<serverID_t, TSession*, NumberHash<serverID_t>> itor( *m_pmapConnectedServerList );
		for( itor.Begin(); itor.IsContinuous(); )
		{
			itor.CurrentItem( &serverID, &pOtherServer );
			if( pOtherServer == NULL )
			{
				itor.Next();
				m_pmapConnectedServerList->Remove( serverID );
				continue;
			}

			itor.Next();
			pOtherServer->EndP();
			m_pmapConnectedServerList->Remove( serverID );
			ReturnTServerToPool( pOtherServer );
		}
	}

	while( m_listUsableTServerElem.empty() == false )
	{
		pOtherServer = m_listUsableTServerElem.front();
		m_listUsableTServerElem.pop_front();

		if( pOtherServer != NULL )
		{
			pOtherServer->EndP();
			ReturnTServerToPool( pOtherServer );
		}
	}

	if( m_pActiveTServerElem->Count() > 0 )
	{
		ULONG_PTR adrTSession = 0;
		TSession* pOtherServer = NULL;

		MapIterator<ULONG_PTR, TSession*, NumberHash<ULONG_PTR>> itor( *m_pActiveTServerElem );
		for( itor.Begin(); itor.IsContinuous(); )
		{
			itor.CurrentItem( &adrTSession, &pOtherServer );
			if( pOtherServer == NULL )
			{
				itor.Next();
				m_pActiveTServerElem->Remove( adrTSession );
				continue;
			}

			itor.Next();
			pOtherServer->EndP();
			ReturnTServerToPool( pOtherServer );
		}
	}

	SAFE_DELETE( m_pmapConnectedServerList );
	SAFE_DELETE( m_pActiveTServerElem );
	SAFE_DELETE( m_pInactiveTServerElem );
}

bool CClientTcp::CreateClientConnection( SOCKET ListenSocket )
{
	if( ListenSocket == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Parameter(%#p) is invalid." ), ListenSocket );
		return false;
	}

	m_pInactiveTClientElem = new TFixedMemoryPool<TClient, MAX_USER_IN_SESSIONSERVER>;
	if( m_pInactiveTClientElem == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to create the objects as TFixedMemoryPool type." ) );
		return false;
	}

	m_pActiveTClientElem = new TMap<ULONG_PTR, TClient*, NumberHash<ULONG_PTR>>;
	if( m_pActiveTClientElem == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to create the object as TMap type." ) );
		SAFE_DELETE( m_pInactiveTClientElem );
		return false;
	}

	m_pmapConnectedClientList = new TMap<objectHandle_t, TClient*, NumberHash<objectHandle_t>>;
	if( m_pmapConnectedClientList == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to create the object as TMap type." ) );
		SAFE_DELETE( m_pInactiveTClientElem );
		SAFE_DELETE( m_pActiveTClientElem );
		return false;
	}

	U16 index = 0;
	TClient* pTClient = NULL;
	for( index = 0; index < MAX_USER_IN_SESSIONSERVER; ++index )
	{
		pTClient = GetTClientFromPool();
		if( pTClient == NULL )
		{
			ReleaseClientConnection();
			return false;
		}

		if( ( pTClient->InitAcpt( this, index, ListenSocket ) ) == false )
		{
			TLOG( LOG_DEBUG, _T( "[ERROR] Failed to initialize the raw object to accepted state." ) );
			ReleaseClientConnection();
			return false;
		}

		pTClient = NULL;
	}

	return true;
}

TClient* CClientTcp::GetTClientFromPool( VOID )
{
	TClient* pTClient = NULL;

	pTClient = m_pInactiveTClientElem->AllocateElement();
	if( pTClient == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to extract a raw object of TClient type." ) );
		return NULL;
	}
	m_pActiveTClientElem->Add( ( ULONG_PTR )pTClient, pTClient );

	return pTClient;
}

bool CClientTcp::ReturnTClientToPool( TClient* pTClient )
{
	m_pInactiveTClientElem->ReturnElement( pTClient );
	return m_pActiveTClientElem->Remove( ( ULONG_PTR )pTClient );
}

VOID CClientTcp::ReleaseClientConnection( VOID )
{
	if( m_pInactiveTClientElem == NULL || m_pActiveTClientElem == NULL || m_pmapConnectedClientList == NULL )
		return;

	if( m_pmapConnectedClientList->Count() > 0 )
	{
		objectHandle_t GOID = 0;
		TClient* pTClient = NULL;

		MapIterator<objectHandle_t, TClient*, NumberHash<objectHandle_t>> itor( *m_pmapConnectedClientList );
		for( itor.Begin(); itor.IsContinuous(); )
		{
			itor.CurrentItem( &GOID, &pTClient );
			if( pTClient == NULL )
			{
				itor.Next();
				m_pmapConnectedClientList->Remove( GOID );
				continue;
			}

			itor.Next();
			pTClient->ReleaseAcpt();
			m_pmapConnectedClientList->Remove( GOID );
			ReturnTClientToPool( pTClient );
		}
	}

	if( m_pActiveTClientElem->Count() > 0 )
	{
		ULONG_PTR adrTClient = 0;
		TClient* pTClient = NULL;

		MapIterator<ULONG_PTR, TClient*, NumberHash<ULONG_PTR>> itor( *m_pActiveTClientElem );
		for( itor.Begin(); itor.IsContinuous(); )
		{
			itor.CurrentItem( &adrTClient, &pTClient );
			if( pTClient == NULL )
			{
				itor.Next();
				m_pActiveTClientElem->Remove( adrTClient );
				continue;
			}

			itor.Next();
			pTClient->ReleaseAcpt();
			ReturnTClientToPool( pTClient );
		}
	}

	SAFE_DELETE( m_pmapConnectedClientList );
	SAFE_DELETE( m_pActiveTClientElem );
	SAFE_DELETE( m_pInactiveTClientElem );
}

bool CClientTcp::BreakListenSocket( VOID )
{
	if( EndpListen() == false )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to release the listen socket." ) );
		return false;
	}

	if( m_pActiveTClientElem->Count() > 0 )
	{
		ULONG_PTR adrTClient = 0;
		TClient* pTClient = NULL;

		MapIterator<ULONG_PTR, TClient*, NumberHash<ULONG_PTR>> itor( *m_pActiveTClientElem );
		for( itor.Begin(); itor.IsContinuous(); )
		{
			itor.CurrentItem( &adrTClient, &pTClient );
			if( pTClient == NULL )
			{
				itor.Next();
				m_pActiveTClientElem->Remove( adrTClient );
				continue;
			}

			itor.Next();

			if( m_pmapConnectedClientList->Find( pTClient->GetGOID(), &pTClient ) == false )
			{
				pTClient->ReleaseAcpt();
				ReturnTClientToPool( pTClient );
			}

			// NOTE: 연결을 받을 수 없는 무의미한 TClient 객체의 발생 가능성
			//		 m_pmapConnectedClientList에 적용된 다중 스레드 동기화 방법과 다중 스레드 영향을 거의
			//		 받지 않도록 사용된 m_pActiveTClientElem에 의해 무의미한 TClient 객체가 쌓일 수 있다.
			//		 위의 m_pmapConnectedClientList->Find()을 통해 대상 TClient 객체가 리슨 소켓 재생과
			//		 관련된 처리를 받아야 하는지 아닌지 구분하고 있는데 다른 스레드에 의해서 간섭 받게 되면
			//		 Find() 처리할 때는 m_pmapConnectedClientList에 존재한다고 판단했으나 Find() 처리를
			//		 끝낸 직후 해당 TClient 객체가 m_pmapConnectedClientList에서 제거될 수 있다. 그럼 이
			//		 TClient 객체는 리슨 소켓 재생과 관련된 처리를 받지 못해 어떤 연결도 받을 수 없게 된다.
		}
	}

	return true;
}
