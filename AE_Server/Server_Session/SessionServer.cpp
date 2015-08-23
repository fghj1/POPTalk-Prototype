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
	// �������� ��ġ�� ��ŷ ���丮�� �����մϴ�.
	::SetCurrentDirectoryW( CGameConfig::Instance()->getRootPath().c_str() );

	m_pTcpClient = new CClientTcp();
	if( m_pTcpClient == false )  // ����! class TAuto_Ptr�� operator bool() const {}�� ȣ���ϰ� �ǹǷ� NULL �񱳰� �ƴ϶� bool �񱳰� �ȴ�.
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
	if( m_pTcpClient == false )  // ����! class TAuto_Ptr�� operator bool() const {}�� ȣ���ϰ� �ǹǷ� NULL �񱳰� �ƴ϶� bool �񱳰� �ȴ�.
	{
		TLOG( LOG_DEBUG, _T( "[WARNING] Do not create the object as CClientTcp type." ) );
		return;
	}

	m_pTcpClient->DisplayConnectedServerNetworkInfo();
}

VOID CSessionServer::RebirthOfListenSocket( VOID )
{
	if( m_pTcpClient == false )  // ����! class TAuto_Ptr�� operator bool() const {}�� ȣ���ϰ� �ǹǷ� NULL �񱳰� �ƴ϶� bool �񱳰� �ȴ�.
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
	// TODO: �ϰ� #157 ����, ���� ���� ó�� �۾��� �� �κе� ���� �����ų �� �ֵ��� �Ѵ�.

	DWORD dwCurtTick = 0, dwUpdateObjTick = timeGetTime(), dwUpdateCCUTick = 0;
	bool bIsUpdate = false;

	m_bMainThreadFinished = false;

	while( TRUE )
	{
		UnitNode stUnitNode;

		dwCurtTick = timeGetTime();

		// ����! m_SessionSMainQ.Size()�� ����Ȯ�� ���
		//		 if( m_SessionSMainQ.Dequeue( stUnitNode ) ) ��ſ� if( 0 < m_SessionSMainQ.Size() )����
		//		 �� ���� ���� �Ǵܽ�, m_SessionSMainQ.Size()�� ����Ȯ���� �ݵ�� �����ؾ� �Ѵ�.
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
			// ������� ���� ��� ��Ŷ ó��
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

	if( m_pTcpClient == false )  // ����! class TAuto_Ptr�� operator bool() const {}�� ȣ���ϰ� �ǹǷ� NULL �񱳰� �ƴ϶� bool �񱳰� �ȴ�.
	{
		TLOG( LOG_DEBUG, _T( "[WARNING] 'CClientTcp' type object is invalid in the %dth shutdown phase." ), m_eNextShutdownPhase );
		return;
	}

	if( 0 < m_nDelayCounter )  // NOTE: ���� ��Ȳ�� ���� ���� �� �ִ� �������� ��
	{
		--m_nDelayCounter;
		return;
	}

	if( 0 < m_nRepeatCounter )  // NOTE: ���� ��Ȳ�� ���� ���� �� �ִ� �������� ��
		--m_nRepeatCounter;

	switch( m_eNextShutdownPhase )
	{
	case SHUTDOWNPHASE_START:
		m_nDelayCounter = 0;
		m_eNextShutdownPhase = BLOCK_NEWCONNECTION;
		m_nRepeatCounter = 3;
		break;

	case BLOCK_NEWCONNECTION:  // ���ο� ���� ����
		_tprintf_s( _T( "Blocking the new connection...\n" ) );
		if( m_pTcpClient->EndpListen() == true )
			m_nRepeatCounter = 0;
		// NOTE: EndpListen() ȣ�� ����, ERROR_OPERATION_ABORTED �߻�
		//		 CIocp::WorkerThreadFunc()���� ERROR_OPERATION_ABORTED�� �߻��Ͽ� ������ ���� �޽����� �ݺ��ؼ� ��µ� �� �ִ�.
		//				[WARNING] The I/O operation has been aborted because of an application request.
		//		 IOCP�� ��ϵ� Listen ���ϰ� �����Ǿ� Accept�� ����ϰ� �ִ� ������ ���� ��ü(TClient)���� ������ŭ �޽����� �ݺ� ��µȴ�.
		//		 Listen ������ ������� �ʴ� �������� ERROR_OPERATION_ABORTED�� �߻����� �ʴ´�.

		if( m_nRepeatCounter <= 0 )
		{
			m_nDelayCounter = 0;
			m_eNextShutdownPhase = HALFCLOSE_CLIENT;
			m_nRepeatCounter = 3;
		}
		break;

	case HALFCLOSE_CLIENT:  // Ŭ���̾�Ʈ �ڿ� ����
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

	case HALFCLOSE_SERVER:  // Ÿ ���� �ڿ� ����
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

	case FINISH_WORKTHREADIOCP_NETWORK:  // ��Ʈ��ũ ó���� ���� IOCP�� �۾� ������ ����� IOCP ���
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

	case FINISH_MAINTHREAD:  // ���� ������ ����
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

	case RELEASE_RESOURCE:  // ���� �ڿ� ����
		_tprintf_s( _T( "Cleaning up server resources...\n" ) );
		m_pTcpClient->End();
		m_eNextShutdownPhase = FINISH_ENTRYPOINTTHREAD;
		break;

	case FINISH_ENTRYPOINTTHREAD:  // main() �Լ� ������ ����
		_tprintf_s( _T( "Terminating the entry point thread...\n" ) );
		m_bMainThreadFinished = true;
		break;

	default:
		TLOG( LOG_DEBUG, _T( "[WARNING] Unknown value(%d)." ), m_eNextShutdownPhase );
		m_eNextShutdownPhase = SHUTDOWNPHASE_START;
		break;
	}
}
