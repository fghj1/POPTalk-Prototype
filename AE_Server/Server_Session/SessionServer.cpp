#include "stdafx.h"
#include "./SessionServer.h"


unsigned __stdcall StartMainThread( LPVOID pOwner )
{
	return ( ( CSessionServer* )pOwner )->SessionSMainThread();
}

CSessionServer::CSessionServer( VOID ):m_unnProcTick( 50 ),
									   m_hSessionSMainThread( 0 ),
									   m_bFinishServer( FALSE ),
									   m_nDelayCounter( 0 ), m_eNextShutdownPhase( SHUTDOWNPHASE_START ), m_nRepeatCounter( 0 ),
									   m_bFinishMainThread( false ), m_bMainThreadFinished( false )
{
}

bool CSessionServer::Begin( VOID )
{
	// 실행파일 위치를 워킹 디렉토리로 설정합니다.
	::SetCurrentDirectoryW( CGameConfig::Instance()->getRootPath().c_str() );

	m_pTcpClient = new CClientTcp();
	if( m_pTcpClient == false )  // 주의! class TAuto_Ptr에 operator bool() const {}을 호출하게 되므로 NULL 비교가 아니라 bool 비교가 된다.
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to create the object as CClientTcp type." ) );
		return false;
	}

	if( m_pTcpClient->Begin() == false )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to start the thread for clients." ) );
		return false;
	}

	m_bFinishMainThread = false;

	m_hSessionSMainThread = ( HANDLE )( _beginthreadex( NULL, 0, StartMainThread, ( LPVOID )this, 0, NULL ) );
	if( NULL == m_hSessionSMainThread )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] ErrNo.%d. Failed to start the main thread of Prototype Server." ), errno );
		return false;
	}

	return true;
}

VOID CSessionServer::DisplayConnectedServerNetworkInfo( VOID )
{
	if( m_pTcpClient == false )  // 주의! class TAuto_Ptr에 operator bool() const {}을 호출하게 되므로 NULL 비교가 아니라 bool 비교가 된다.
	{
		TLOG( LOG_DEBUG, _T( "[WARNING] Do not create the object as CClientTcp type." ) );
		return;
	}

	m_pTcpClient->DisplayConnectedServerNetworkInfo();
}

VOID CSessionServer::RebirthOfListenSocket( VOID )
{
	if( m_pTcpClient == false )  // 주의! class TAuto_Ptr에 operator bool() const {}을 호출하게 되므로 NULL 비교가 아니라 bool 비교가 된다.
	{
		TLOG( LOG_DEBUG, _T( "[WARNING] Do not create the object as CClientTcp type." ) );
		return;
	}

	if( m_pTcpClient->RebirthOfListenSocket() == false )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to revive the listen socket. Prototype Server is shutting down. Please wait..." ) );
		SetbFinishServer( TRUE );
	}
}

VOID CSessionServer::GetSizeSessionSMainQ( VOID )
{
#ifdef WIN64
	_tprintf_s( _T( "\nSize of the Main Q : %I64d\n" ), m_SessionSMainQ.Size() );
#else
	_tprintf_s( _T( "\nSize of the Main Q : %I32d\n" ), m_SessionSMainQ.Size() );
#endif
}

VOID CSessionServer::ChangeUpdateCycle( VOID )
{
	UINT unnProcTimes = 0, unnAnswer = 0;

	unnProcTimes = 1000 / m_unnProcTick;
	_tprintf_s( _T( "\n\nHow many times per second does SessionSMainThread() process 'Update Part'?\n(curt. %d t/s)\n" ), unnProcTimes );
	_tscanf_s( _T( "%d" ), &unnAnswer );
	if( unnAnswer < 1 || 1000 < unnAnswer )
		_tprintf_s( _T( "input value out of range(1 <= input <= 1000).\nPlease enter a value by pressing the 'Ctrl+T'key again.\n" ) );
	else
	{
		m_unnProcTick = 1000 / unnAnswer;
		_tprintf_s( _T( "'Update Part' of SessionSMainThread() processes %d times per second.\n" ), unnAnswer );
	}
}

UINT CSessionServer::SessionSMainThread( VOID )
{
	// TODO: 일감 #157 관련, 서버 종료 처리 작업시 이 부분도 같이 종료시킬 수 있도록 한다.

	DWORD dwCurtTick = 0, dwUpdateObjTick = timeGetTime(), dwUpdateCCUTick = 0;
	bool bIsUpdate = false;

	m_bMainThreadFinished = false;

	while( TRUE )
	{
		UnitNode stUnitNode;

		dwCurtTick = timeGetTime();

		// 주의! m_SessionSMainQ.Size()의 부정확성 경고
		//		 if( m_SessionSMainQ.Dequeue( stUnitNode ) ) 대신에 if( 0 < m_SessionSMainQ.Size() )으로
		//		 블럭 진입 여부 판단시, m_SessionSMainQ.Size()의 부정확성을 반드시 개선해야 한다.
		while( m_SessionSMainQ.Dequeue( stUnitNode ) )
		{
			if( stUnitNode.pHostOfReceivedData )
			{
				if( stUnitNode.bIsClient )
					( ( TClient* )( stUnitNode.pHostOfReceivedData ) )->PacketProcess( stUnitNode.ulReceivedData );
				else
					( ( TSession* )( stUnitNode.pHostOfReceivedData ) )->PacketProcess( stUnitNode.ulReceivedData );
			}

			if( m_unnProcTick < GetDeltaTime( dwUpdateObjTick, dwCurtTick ) )
			{
				bIsUpdate = true;
				break;
			}
		}

		if( bIsUpdate )
		{
			if( 17000 < GetDeltaTime( dwUpdateCCUTick, dwCurtTick ) )
			{
				if( m_pTcpClient )
					m_pTcpClient->UpdateCCU();

				dwUpdateCCUTick = dwCurtTick;
			}
			m_pTcpClient->ServerSessShutdown( false );

			dwUpdateObjTick = dwCurtTick;
			bIsUpdate = false;
		}
		else
		{
			if( m_unnProcTick < GetDeltaTime( dwUpdateObjTick, dwCurtTick ) )
				bIsUpdate = true;
			else
				Sleep( 1 );
		}

		if( m_bFinishMainThread == true )
		{
			// 현재까지 받은 모든 패킷 처리
			while( m_SessionSMainQ.Dequeue( stUnitNode ) )
			{
				if( stUnitNode.pHostOfReceivedData )
				{
					if( stUnitNode.bIsClient )
						( ( TClient* )( stUnitNode.pHostOfReceivedData ) )->PacketProcess( stUnitNode.ulReceivedData );
					else
						( ( TSession* )( stUnitNode.pHostOfReceivedData ) )->PacketProcess( stUnitNode.ulReceivedData );
				}
			}

			break;
		}
	}

	return 0;
}

VOID CSessionServer::InsertIntoMainQ( bool bIsClient, LPVOID pHostOfReceivedData, ULONG_PTR ulReceivedData )
{
	UnitNode stUnitNode;
	stUnitNode.bIsClient = bIsClient;
	stUnitNode.pHostOfReceivedData = pHostOfReceivedData;
	stUnitNode.ulReceivedData = ulReceivedData;

	m_SessionSMainQ.Enqueue( stUnitNode );
}

VOID CSessionServer::ShutdownSequence( VOID )
{
	if( m_bFinishServer == FALSE )
		return;

	if( m_pTcpClient == false )  // 주의! class TAuto_Ptr에 operator bool() const {}을 호출하게 되므로 NULL 비교가 아니라 bool 비교가 된다.
	{
		TLOG( LOG_DEBUG, _T( "[WARNING] 'CClientTcp' type object is invalid in the %dth shutdown phase." ), m_eNextShutdownPhase );
		return;
	}

	if( 0 < m_nDelayCounter )  // NOTE: 서버 상황에 따라 변할 수 있는 경험적인 값
	{
		--m_nDelayCounter;
		return;
	}

	if( 0 < m_nRepeatCounter )  // NOTE: 서버 상황에 따라 변할 수 있는 경험적인 값
		--m_nRepeatCounter;

	switch( m_eNextShutdownPhase )
	{
	case SHUTDOWNPHASE_START:
		m_nDelayCounter = 0;
		m_eNextShutdownPhase = BLOCK_NEWCONNECTION;
		m_nRepeatCounter = 3;
		break;

	case BLOCK_NEWCONNECTION:  // 새로운 연결 차단
		_tprintf_s( _T( "Blocking the new connection...\n" ) );
		if( m_pTcpClient->EndpListen() == true )
			m_nRepeatCounter = 0;
		// NOTE: EndpListen() 호출 직후, ERROR_OPERATION_ABORTED 발생
		//		 CIocp::WorkerThreadFunc()에서 ERROR_OPERATION_ABORTED가 발생하여 다음과 같은 메시지가 반복해서 출력될 수 있다.
		//				[WARNING] The I/O operation has been aborted because of an application request.
		//		 IOCP에 등록된 Listen 소켓과 연관되어 Accept를 대기하고 있는 사용되지 않은 객체(TClient)들의 개수만큼 메시지가 반복 출력된다.
		//		 Listen 소켓을 사용하지 않는 곳에서는 ERROR_OPERATION_ABORTED가 발생하지 않는다.

		if( m_nRepeatCounter <= 0 )
		{
			m_nDelayCounter = 0;
			m_eNextShutdownPhase = HALFCLOSE_CLIENT;
			m_nRepeatCounter = 3;
		}
		break;

	case HALFCLOSE_CLIENT:  // 클라이언트 자원 정리
		_tprintf_s( _T( "Cleaning up client resources...\n" ) );
		m_pTcpClient->ClientSessShutdown();
		if( m_pTcpClient->GetCCU() <= 0 )
			m_nRepeatCounter = 0;
		else
			m_nDelayCounter = 2;

		if( m_nRepeatCounter <= 0 )
		{
			m_nDelayCounter = 0;
			m_eNextShutdownPhase = HALFCLOSE_SERVER;
			m_nRepeatCounter = 5;
		}
		break;

	case HALFCLOSE_SERVER:  // 타 서버 자원 정리
		_tprintf_s( _T( "Cleaning up other server resources...\n" ) );
		m_pTcpClient->ServerSessShutdown();
		if( m_pTcpClient->GetConcurrentServer() <= 0 )
			m_nRepeatCounter = 0;
		else
			m_nDelayCounter = 10;

		if( m_nRepeatCounter <= 0 )
		{
			m_nDelayCounter = 0;
			m_eNextShutdownPhase = FINISH_WORKTHREADIOCP_NETWORK;
			m_nRepeatCounter = 3;
		}
		break;

	case FINISH_WORKTHREADIOCP_NETWORK:  // 네트워크 처리를 위한 IOCP의 작업 스레드 종료와 IOCP 폐쇄
		_tprintf_s( _T( "Cleaning up network IOCP resources...\n" ) );
		if( m_pTcpClient->CloseWorkThreadAndIOCP() == true )
			m_nRepeatCounter = 0;

		if( m_nRepeatCounter <= 0 )
		{
			m_nDelayCounter = 0;
			m_eNextShutdownPhase = FINISH_MAINTHREAD;
			m_nRepeatCounter = 3;
		}
		break;

	case FINISH_MAINTHREAD:  // 메인 스레드 종료
		_tprintf_s( _T( "Terminating the main thread...\n" ) );
		m_bFinishMainThread = true;
		{
			DWORD dwResult = WaitForSingleObject( m_hSessionSMainThread, 122 );
			if( dwResult == WAIT_OBJECT_0 )
			{
				if( m_hSessionSMainThread != 0 )
				{
					if( CloseHandle( m_hSessionSMainThread ) == FALSE )
						TLOG( LOG_DEBUG, _T( "[WARNING] ErrNo.%d. Failed to close the main thread handle of Prototype Server." ), GetLastError() );
					else
						m_hSessionSMainThread = 0;
				}

				m_nRepeatCounter = 0;
			}
			else if( dwResult != WAIT_TIMEOUT )
			{
				TLOG( LOG_DEBUG, _T( "[WARNING] ErrNo.%d. Failed to wait until the specified object is in the signaled state." ), GetLastError() );
				m_nDelayCounter = 5;
			}
		}

		if( m_nRepeatCounter <= 0 )
		{
			m_nDelayCounter = 0;
			m_eNextShutdownPhase = RELEASE_RESOURCE;
			m_nRepeatCounter = 3;
		}
		break;

	case RELEASE_RESOURCE:  // 각종 자원 해제
		_tprintf_s( _T( "Cleaning up server resources...\n" ) );
		m_pTcpClient->End();
		m_eNextShutdownPhase = FINISH_ENTRYPOINTTHREAD;
		break;

	case FINISH_ENTRYPOINTTHREAD:  // main() 함수 스레드 종료
		_tprintf_s( _T( "Terminating the entry point thread...\n" ) );
		m_bMainThreadFinished = true;
		break;

	default:
		TLOG( LOG_DEBUG, _T( "[WARNING] Unknown value(%d)." ), m_eNextShutdownPhase );
		m_eNextShutdownPhase = SHUTDOWNPHASE_START;
		break;
	}
}
