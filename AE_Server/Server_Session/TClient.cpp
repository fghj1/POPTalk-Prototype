#include "stdafx.h"
#include "./TClient.h"
#include "./ClientTcp.h"
#include "./SessionServer.h"


TClient::TClient( VOID ):CPeer( NETWORK_KIND_CLIENT ),
						 m_pClientTcp( NULL ), m_bIsConnecting( false ),
						 m_GOID( 0 ),
						 m_affiliatedGameServerID( 0 ),
						 m_bStopSend( false )
{
}

bool TClient::InitAcpt( CClientTcp* pClientTcp, U16 creationNo, SOCKET ListenSocket )
{
	if( pClientTcp == NULL )
		return false;

	m_pClientTcp = pClientTcp;
	SetIsConnecting( false );
	m_creationNo = creationNo;
	m_bStopSend = false;

	// NOTE: m_creationNo에 최초 할당된 값 유지
	//		 CClientTcp::RebirthOfListenSocket()에서도 m_creationNo의 최초 할당 값은 그대로 재사용 되어야
	//		 합니다. 그렇지 않으면 CClientTcp::RebirthOfListenSocket()에 의해 처리되지 않은 객체,
	//		 즉 리슨 소켓이 필요없는 이미 연결 중인 객체가 가진 m_creationNo 값과 중복될 수 있습니다.
	//		 따라서 m_creationNo은 초기화되어선 안됩니다.

	if( CPeer::StartP() == FALSE )
		return false;

	if( CNetwork::Accept( ListenSocket ) == FALSE )
		return false;

	return true;
}

BOOL TClient::ReleaseAcpt( VOID )
{
	m_pClientTcp = NULL;
	SetIsConnecting( false );
	SetGOID( 0 );
	m_bStopSend = false;

	return CPeer::EndP();
}

bool TClient::ReinitAcpt( SOCKET ListenSocket )
{
	SCOPED_LOCK( m_CS )
	{
		SetIsConnecting( false );
		SetGOID( 0 );
		m_bStopSend = false;

		if( CPeer::EndP() == FALSE )
			return false;

		if( CPeer::StartP() == FALSE )
			return false;

		if( CNetwork::Accept( ListenSocket ) == FALSE )
			return false;
	}

	return true;
}

VOID TClient::InduceSafeDisconnection( __in INT iHow/* = SD_RECEIVE*/ )
{
	if( CNetwork::HalfClose( iHow ) == false )
		TLOG( LOG_DEBUG, _T( "[WARNING] Failed to call the HalfClose() for client(%I64d) socket." ), GetGOID() );

	m_pClientTcp->PostQueuedCompletionStatus( 0, reinterpret_cast<ULONG_PTR>( this ), &( GetstReceiveOverlappedEX().Overlapped ) );

	m_bStopSend = true;
}

bool TClient::WritePacket( __in WORD wSendSize/* = 0*/, __in BYTE* pbySendData/* = NULL*/ )
{
	if( m_bStopSend == true )
		return false;

	size_t nSizeUnprocessedWritePD = GetSizeUnprocessedWritePD();

	if( nSizeUnprocessedWritePD > 500 )
	{
		TLOG( LOG_DEBUG,
			  _T( "[ERROR] There are too many packets(%Id) that can not be sent to the client(%I64d). Disconnecting with the client." ),
			  nSizeUnprocessedWritePD, GetGOID() );
		InduceSafeDisconnection();
	}
	else
	{
		if( GetIsConnecting() && ( CPeer::WritePacket( wSendSize, pbySendData ) < 0 ) )
		{
			TLOG( LOG_DEBUG,
				  _T( "[ERROR] Failed to send packet to client(%I64d). Disconnecting with the client." ),
				  GetGOID() );
			InduceSafeDisconnection();
		}
	}

	return false;
}

bool TClient::OnIOConnected( VOID )
{
	SCOPED_LOCK( m_CS )
	{
		SOCKET uni64NSocket = GetNSocket();
		SOCKET listSocket = m_pClientTcp->GetListenSocket();

		if( setsockopt( uni64NSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<char*>( &listSocket ), sizeof( listSocket ) ) == SOCKET_ERROR )
		{
			TLOG( LOG_DEBUG, _T( "[ERROR] Failed to set a socket option(SO_UPDATE_ACCEPT_CONTEXT). ErrNo.%d" ), WSAGetLastError() );
			return false;
		}

		if( m_pClientTcp->RegisterSocketToIocp( uni64NSocket, reinterpret_cast<ULONG_PTR>( this ) ) == false )
			return false;

		SetIsConnecting( true );

		UID GOID( 1, 1, m_creationNo );
		SetGOID( GOID.dwKey );

		m_pClientTcp->AddConnectedClient( GetGOID(), this );

		if( ReadPacketForIOCP() == false )
		{ 
			OnIODisconnected();
			return false; 
		}

		S2C_ONCLIENTCONNECT_CMD proto;

		TSPacket cpkRegister;
		cpkRegister.protocol( proto.Tag );
		proto.objHandle = 0;
		proto.Write( &cpkRegister );
		if( WritePacket( cpkRegister.Length(), cpkRegister.getBuffer() ) == false )
			return false;
	}

	return true;
}

VOID TClient::OnIODisconnected( VOID )
{
	// NOTE: 본 함수가 연속해서 두번 호출될 수 있습니다.
	//		 Redmine #2648 '패킷 프로토콜 불일치로 인해 영지 모드 서버가 비정상 종료되는 문제' 참고
	if( GetGOID() )
	{
		m_pClientTcp->RemoveConnectedClient( GetGOID() );
		SetIsConnecting( false );
		TLOG( LOG_DEBUG, _T( "Client(%I64d) connection terminated." ), GetGOID() );

		ReinitAcpt( m_pClientTcp->GetListenSocket() );
		TLOG( LOG_DEBUG, _T( "Connect count : %d" ), m_pClientTcp->GetCCU() );
	}
}

VOID TClient::OnIORead( U32 dwDataLength )
{
	bool bResult = false;

	if( dwDataLength != 0 )
		bResult = ReadPacketForIOCP( dwDataLength );

	if( bResult == false )
	{
		bResult = InsertEOFDataIntoMainQ();
		if( bResult == false )
			OnIODisconnected();
	}
}

bool TClient::ChangePacketProcessingMode( __inout BYTE* pbyData )
{
	WORD wSize = 0, wProtocol = 0;
	TSPacket pack;

	wSize = MAKEWORD( *pbyData, *( pbyData + 1 ) );
	pack.copyToBuffer( pbyData, wSize );
	wProtocol = pack.protocol();

	tPacketHandler handler;
	handler = ( m_pClientTcp->GetPacketHandlerMGR() ).GetHandler( wProtocol );
	if( handler.empty() )
	{
		eServerType toDestinationServer = SERVER_NONE;
		pack.userID( GetGOID() );

		toDestinationServer = ( m_pClientTcp->GetPacketHandlerMGR() ).GetDestination( wProtocol );
		switch( toDestinationServer )
		{
		default:
			TLOG( LOG_DEBUG, _T( "[WARNING] Unknown protocol ID : %d" ), wProtocol );
			InduceSafeDisconnection();
			break;
		}

		return true;
	}

	return false;
}

bool TClient::ThrowToMainQ( __in ULONG_PTR ulReceivedData )
{
	SER()->InsertIntoMainQ( true, this, ulReceivedData );
	return false;
}

bool TClient::InsertEOFDataIntoMainQ( VOID )
{
	if( GetIsConnecting() )
	{
		CnS_DISCONNECT_CMD protoCnSCMD;
		TSPacket EOFPack;

		EOFPack.protocol( protoCnSCMD.Tag );
		protoCnSCMD.Write( &EOFPack );

		if( CPeer::InsertREQDataIntoMainQ( EOFPack.Length(), EOFPack.getBuffer() ) == false )
			return false;

		SetIsConnecting( false );
	}
	else
		return false;  // NOTE: Redmine #2648 '패킷 프로토콜 불일치로 인해 영지 모드 서버가 비정상 종료되는 문제' 참고

	return true;
}

void TClient::PacketProcess( ULONG_PTR ulProcessData )
{
	if( m_pClientTcp == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Management object(%#p) is invalid." ), m_pClientTcp );
		return;
	}

	BYTE byPacket[PACKET_BUFFER_SIZE] = {0,};
	WORD wSize = 0, wProtocol = 0;
	TSPacket pack;

	try
	{
		if( ReadPacketForIOCP( _countof( byPacket ), byPacket, ulProcessData ) )
		{
			wSize = MAKEWORD( byPacket[0], byPacket[1] );
			pack.copyToBuffer( byPacket, wSize );
			wProtocol = pack.protocol();

			tPacketHandler handler;
			handler = ( m_pClientTcp->GetPacketHandlerMGR() ).GetHandler( wProtocol );
			if( handler.empty() == false )
				handler( this, &pack, NULL );
			else
				TLOG( LOG_DEBUG, _T( "[WARNING] Unknown protocol ID : %d" ), wProtocol );
		}
	}
	catch( std::exception& e )
	{
		std::tcout << e.what() << std::endl;
	}
}
