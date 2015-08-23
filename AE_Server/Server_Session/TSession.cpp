#include "stdafx.h"
#include "./TSession.h"
#include "./ClientTcp.h"
#include "./TClient.h"
#include "./SessionServer.h"


TSession::TSession( VOID ):CPeer( NETWORK_KIND_SERVER ), m_pClientTcp( NULL ),
						   m_bIsConnecting( false ), m_bWaitingToDisconnect( false )
{
}

bool TSession::Reinit( VOID )
{
	SCOPED_LOCK( m_CS )
	{
		SetIsConnecting( false );
		SetWaitingToDisconnect( false );
		m_serverID.value = 0;

		if( CPeer::EndP() == FALSE )
			return false;

		if( CPeer::StartP() == FALSE )
			return false;
	}

	return true;
}

VOID TSession::Disconnect( VOID )
{
	SCOPED_LOCK( m_CS )
	{
		if( GetIsConnecting() )
		{
			m_pClientTcp->DisconnectedServer( this );

			TLOG( LOG_DEBUG, _T( "Server(%d) connection terminated." ), m_serverID.value );

			Reinit();
			m_pClientTcp->ReusableSession( this );
		}

		// TODO: #1233 WS가 죽은 경우 SS의 대응 처리
		//		 if( GetIsConnecting() ) 블럭 안에 넣을지 밖에 둘지는 대응 처리 상황에 따라 재판단해야 한다.
	}
}

VOID TSession::InduceSafeDisconnection( VOID )
{
	m_pClientTcp->PostQueuedCompletionStatus( 0, reinterpret_cast<ULONG_PTR>( this ), &( GetstReceiveOverlappedEX().Overlapped ) );
}

bool TSession::OnIOConnected( VOID )
{
	SCOPED_LOCK( m_CS )
	{
		if( m_pClientTcp->RegisterSocketToIocp( GetNSocket(), reinterpret_cast<ULONG_PTR>( this ) ) == false )
			return false;

		SetIsConnecting( true );

		if( ReadPacketForIOCP() == false )
		{ 
			Disconnect();
			return false; 
		}
	}

	return true;
}

VOID TSession::OnIODisconnected( VOID )
{
	if( m_bIsSD_SEND == false )
	{
		if( SER()->GetbFinishServer() == TRUE )  // 내부 종료 결정에 의한 종료
		{
			if( CNetwork::HalfClose( SD_SEND ) == false )
				TLOG( LOG_DEBUG, _T( "[WARNING] Failed to call the HalfClose() for server(%d) socket." ), GetserverID() );
			else
				return;
		}
		else  // 외부 종료 결정에 의한 연동 종료
		{
			if( 0 < GetSizeUnprocessedWritePD() )
			{
				if( InsertEOFDataIntoMainQ() == true )
					return;
			}

			if( CNetwork::HalfClose( SD_SEND ) == false )
				TLOG( LOG_DEBUG, _T( "[WARNING] Failed to call the HalfClose() for server(%d) socket." ), GetserverID() );
		}
	}

	if( 0 < GetSizeUnprocessedPacket() )
	{
		SetWaitingToDisconnect( true );
		return;
	}

	Disconnect();
}

VOID TSession::OnIORead( U32 dwDataLength )
{
	if( GetIsConnecting() )
	{
		bool bResult = false;

		if( dwDataLength != 0 )
			bResult = ReadPacketForIOCP( dwDataLength );

		if( bResult == false )
		{
			bResult = InsertEOFDataIntoMainQ();
			if( bResult == false )
				Disconnect();
		}
	}
}

VOID TSession::OnIOWrote( VOID )
{
	if( GetIsConnecting() && ( WritePacket() < 0 ) )
	{
		TLOG( LOG_DEBUG,
			  _T( "[ERROR] Failed to send packet to server(%d). Disconnecting with the server(%d)." ),
			  m_serverID.value, m_serverID.value );
		Disconnect();
	}
}

bool TSession::ChangePacketProcessingMode( __in BYTE* pbyData )
{
	WORD wSize = 0, wProtocol = 0;
	TSPacket pack;

	wSize = MAKEWORD( *pbyData, *( pbyData + 1 ) );
	pack.copyToBuffer( pbyData, wSize );

	tPacketHandler handler;
	handler = ( m_pClientTcp->GetPacketHandlerMGR() ).GetHandler( pack.protocol() );
	if( handler.empty() )
	{
		TClient* pClient = NULL;
		m_pClientTcp->FindConnectedClient( pack.userID(), &pClient );
		if( pClient != NULL )
			pClient->WritePacket( pack.Length(), pack.getBuffer() );

		return true;
	}

	return false;
}

bool TSession::ThrowToMainQ( __in ULONG_PTR ulReceivedData )
{
	SER()->InsertIntoMainQ( false, this, ulReceivedData );
	return false;
}

void TSession::PacketProcess( ULONG_PTR ulProcessData )
{
	if( m_pClientTcp == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Management object(%#p) is invalid." ), m_pClientTcp );
		return;
	}

	BYTE byPacket[PACKET_BUFFER_SIZE] = {0,};
	WORD wSize = 0, wProtocol = 0;
	TSPacket spkPacket;

	if( GetIsConnecting() && ReadPacketForIOCP( _countof( byPacket ), byPacket, ulProcessData ) )
	{
		wSize = MAKEWORD( byPacket[0], byPacket[1] );
		spkPacket.copyToBuffer( byPacket, wSize );
		wProtocol = spkPacket.protocol();

		tPacketHandler handler;
		handler = ( m_pClientTcp->GetPacketHandlerMGR() ).GetHandler( wProtocol );
		if( handler.empty() == false )
			handler( this, &spkPacket, NULL );
		else
			TLOG( LOG_DEBUG, _T( "[WARNING] Unknown protocol ID : %d" ), wProtocol );
	}
}

bool TSession::InsertEOFDataIntoMainQ( VOID )
{
	TSPacket EOFPack;
	SnS_DISCONNECT_CMD protoSnSCMD;

	EOFPack.protocol( protoSnSCMD.Tag );
	EOFPack.userID( 0 );
	protoSnSCMD.isAllServerShutdown = false;
	protoSnSCMD.Write( &EOFPack );

	if( CPeer::InsertREQDataIntoMainQ( EOFPack.Length(), EOFPack.getBuffer() ) == false )
		return false;

	return true;
}
