#include "NetworkPCH.h"
#include "./Network.h"
#include "./Peer.h"
#include "./WSAEventSelect.h"
#include "./ClientSession.h"


HANDLE CClientSession::m_hOperatingEventOfCS = NULL;

BOOL __stdcall ClientConnectionHandler( DWORD ctrlType )
{
	SetEvent( CClientSession::m_hOperatingEventOfCS );
	return TRUE;
}

CClientSession::CClientSession():m_IsConnection( false )
{
}

CClientSession::~CClientSession()
{
}

bool CClientSession::StartCS( NetAddress& stConnectionAddr )
{
	LPSTR lpszAddr = stConnectionAddr.ToString();  // The size of buffer in NetAddress type : 20

	if( strnlen( lpszAddr, 20 ) >= 20 || stConnectionAddr.port <= 0 )
		return false;

	if( m_Session.StartP() == FALSE )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to initialize the object as CPeer type." ) );
		return false;
	}

	if( m_Session.CreateNBOLSocket() == FALSE )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to create the socket." ) );
		return false;
	}

	if( CWSAEventSelect::WSAESBegin( m_Session.GetNSocket() ) == false )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to create the WSAEventSelect model." ) );
		return false;
	}

	if( sizeof( TCHAR ) == sizeof( WCHAR ) )
	{
		WCHAR szAddress[20] = {0,};

		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, lpszAddr, -1, szAddress, 20 );
		TLOG( LOG_CRT, _T( "Connecting to the %s:%d..." ), szAddress, stConnectionAddr.port );
	}
	else
		TLOG( LOG_CRT, _T( "Connecting to the %s:%d..." ), lpszAddr, stConnectionAddr.port );

	if( m_Session.CreateConnection( lpszAddr, stConnectionAddr.port ) == false )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to connect the opposite side." ) );
		return false;
	}

	m_hOperatingEventOfCS = CreateEvent( NULL, FALSE, FALSE, NULL );
	if( m_hOperatingEventOfCS == NULL )
	{
		DWORD dwErrNo = GetLastError();
		TLOG( LOG_DEBUG, _T( "[ERROR] Need to check. ErrNo.%d" ), dwErrNo );
	}

	SetConsoleCtrlHandler( ClientConnectionHandler, TRUE );
	{
		WaitForSingleObject( m_hOperatingEventOfCS, WAIT_TIMEOUT_INTERVAL );
		CloseHandle( m_hOperatingEventOfCS );
		m_hOperatingEventOfCS = NULL;
	}
	SetConsoleCtrlHandler( ClientConnectionHandler, FALSE );

	if( m_IsConnection == false )
	{
		TLOG( LOG_DEBUG, _T( "Connection failed!!" ) );
		return false;
	}

	return true;
}

bool CClientSession::EndCS( VOID )
{
	CWSAEventSelect::WSAESEnd();
	m_Session.EndP();

	return true;
}

bool CClientSession::ReceivePacket( __out BYTE* pbyReceivedPacket/* = NULL*/ )
{
	// TODO: 서버간 재연결 작업
	//		 처리가 실패하였을 경우, 연결을 끊었다가 다시 연결하도록 처리해야 한다. IOCP에서는 ReinitAcpt()가 이런 역할을 한다.
	return m_Session.ReadPacketForWSAES( pbyReceivedPacket );
}

bool CClientSession::SendPacket( __in WORD wLength/* = 0*/, __in BYTE* pbyBuffer/* = NULL*/ )
{
	if( m_IsConnection )
	{
		// TODO: 서버간 재연결 작업
		//		 처리가 실패하였을 경우, 연결을 끊었다가 다시 연결하도록 처리해야 한다. IOCP에서는 ReinitAcpt()가 이런 역할을 한다.
		if( 0 <= m_Session.WritePacket( wLength, pbyBuffer ) )
			return true;
	}

	return false;
}
