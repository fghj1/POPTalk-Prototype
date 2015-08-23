// Server_Session.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "./SessionServer.h"


LPTOP_LEVEL_EXCEPTION_FILTER g_pfnPrevUnhandledExceptionFilter = NULL;
volatile LONG g_CCU = 0;  // CCU : ���� ������ ��(Concurrent Connection User)

BOOL WINAPI ServerStopHandler( DWORD dwCtrlType );
VOID KeyEventProc( KEY_EVENT_RECORD KeyEventInfo );
VOID DisplayCCU( VOID );

int _tmain( int argc, _TCHAR* argv[] )
{
#if defined( _DEBUG )
//	_CrtSetBreakAlloc( 1856 );
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	{
#endif

	// Server � ������ �ڿ� �ʱ�ȭ
	_tsetlocale( LC_ALL, _T( "korean" ) );

	HANDLE hMutex = NULL;
	DWORD dwErrNo = 0;
	WSADATA stWSAData;
	INT iTermKey = 0, iAnswer = 0;

	// Mutex ����
	hMutex = CreateMutex( NULL, FALSE, _T( "Server_Prototype" ) );
	dwErrNo = GetLastError();
	if( dwErrNo != ERROR_SUCCESS )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] ErrNo.%d" ), dwErrNo );
		if( dwErrNo == ERROR_ACCESS_DENIED || dwErrNo == ERROR_ALREADY_EXISTS )
			TLOG( LOG_DEBUG, _T( "Warning!! Already Exist Prototype Server.\n" ) );
		return 0;
	}

	// WinSock2 lib �ʱ�ȭ
	dwErrNo = WSAStartup( MAKEWORD( 2, 2 ), &stWSAData );
	if( dwErrNo != 0 )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] ErrNo.%d" ), dwErrNo );
		ReleaseMutex( hMutex );
		CloseHandle( hMutex );
		return 0;
	}

	// ���� �߻� �� Dump �߻� Handler ����
	SetErrorMode( SEM_FAILCRITICALERRORS );
	//g_pfnPrevUnhandledExceptionFilter = SetUnhandledExceptionFilter( UnhandledExceptionCallStack );
	g_pfnPrevUnhandledExceptionFilter = LOGF()->initMinidumpFilter();

	// ȯ�� ���� �� �б�
	if( CONFIG()->InitEnvironmentConfig( _T( "Prototype Server" ), SERVER_PROTOTYPE ) == false )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to initialize environment configuration." ) );
		SetUnhandledExceptionFilter( g_pfnPrevUnhandledExceptionFilter );
		WSACleanup();
		ReleaseMutex( hMutex );
		CloseHandle( hMutex );
		return 0;
	}

	// Server � ����
	try
	{
		if( SER()->Begin() )
		{
			TCHAR szConsoleTitle[MAX_PATH] = {0,};
			_stprintf_s( szConsoleTitle, _countof( szConsoleTitle ), _T( "Prototype Server" ) );
			SetConsoleTitle( szConsoleTitle );
			TLOG( LOG_FILE, _T( "Starting Prototype Server.\n" ) );

			HANDLE hStdInput = GetStdHandle( STD_INPUT_HANDLE );
			if( NULL == hStdInput )
			{
				while( false == SER()->GetbMainThreadFinished() )
				{
					// TODO: ������ ����ȭ�� ���� ����� �� ���� �� Ű ��� ����� ���� ���� ������� ����� �� �־�� �Ѵ�.

					SER()->RebirthOfListenSocket();
					SER()->ShutdownSequence();  // Server � ���� ���� ����

					Sleep( 1 );
				}
			}
			else if( INVALID_HANDLE_VALUE == hStdInput )
				TLOG( LOG_DEBUG, _T( "[ERROR] ErrNo.%d. Failed to retrieve a handle to the standard input." ), GetLastError() );
			else
			{
				DWORD dwResult = 0;
				INPUT_RECORD stStdInputInfo;
				DWORD dwNumberOfEventRead = 0;

				if( FALSE == SetConsoleMode( hStdInput, 0 ) )
					TLOG( LOG_DEBUG, _T( "[WARNING] ErrNo.%d. Failed to set the input mode of a console's input buffer." ), GetLastError() );

				// TODO: �� ���� �� ���� X��ư ��Ȱ��ȭ �� ��
				//		 ���⼭ ��Ȱ��ȭ�Ͽ��� �ð��� ������ �ٽ� Ȱ��ȭ �Ǵ� ������ �ִ�.
				//EnableMenuItem( GetSystemMenu( GetConsoleWindow(), FALSE ), SC_CLOSE, MF_GRAYED );
				//DrawMenuBar( GetConsoleWindow() );

				if( FALSE == FlushConsoleInputBuffer( hStdInput ) )
					TLOG( LOG_DEBUG, _T( "[WARNING] ErrNo.%d. Failed to flush the console input buffer." ), GetLastError() );

				SetConsoleCtrlHandler( ServerStopHandler, TRUE );
				while( false == SER()->GetbMainThreadFinished() )
				{
					dwResult = WaitForSingleObject( hStdInput, 3000 );
					if( WAIT_OBJECT_0 == dwResult )
					{
						if( true == SER()->GetbMainThreadFinished() )
							break;

						ZeroMemory( static_cast<LPVOID>( &stStdInputInfo ), sizeof( INPUT_RECORD ) );
						if( 0 == ReadConsoleInput( hStdInput, &stStdInputInfo, 1, &dwNumberOfEventRead ) )
						{
							TLOG( LOG_DEBUG, _T( "[WARNING] ErrNo.%d. Failed to read data from a console input buffer." ), GetLastError() );
							if( FALSE == FlushConsoleInputBuffer( hStdInput ) )
								TLOG( LOG_DEBUG, _T( "[WARNING] ErrNo.%d. Failed to flush the console input buffer." ), GetLastError() );
						}

						if( KEY_EVENT == stStdInputInfo.EventType )
							KeyEventProc( stStdInputInfo.Event.KeyEvent );
					}
					else if( WAIT_FAILED == dwResult && WAIT_ABANDONED == dwResult )
						TLOG( LOG_DEBUG, _T( "[WARNING] ErrNo.%d. The WaitForSingleObject() has failed." ), GetLastError() );

					DisplayCCU();

					SER()->RebirthOfListenSocket();
					SER()->ShutdownSequence();  // Server � ���� ���� ����
				}
				SetConsoleCtrlHandler( ServerStopHandler, FALSE );
			}
		}
	}
	catch( std::exception& e )
	{
		std::tcout << e.what() << std::endl;
	}

	// Server � ������ �ڿ� ����
	SetUnhandledExceptionFilter( g_pfnPrevUnhandledExceptionFilter );

	WSACleanup();

	ReleaseMutex( hMutex );
	CloseHandle( hMutex );

#if defined( _DEBUG )
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

BOOL WINAPI ServerStopHandler( DWORD dwCtrlType )
{
	BOOL bResult = FALSE;

	switch( dwCtrlType )
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		bResult = TRUE;
		break;
	}

	return bResult;
}

VOID KeyEventProc( KEY_EVENT_RECORD KeyEventInfo )
{
	if( TRUE == KeyEventInfo.bKeyDown )
	{
		if( LEFT_CTRL_PRESSED & KeyEventInfo.dwControlKeyState )  // Ctrl key
		{
			switch( KeyEventInfo.wVirtualKeyCode )
			{
			case 0x50:  // P key
				SER()->DisplayConnectedServerNetworkInfo();
				break;

			case 0x53:  // S key
				SER()->GetSizeSessionSMainQ();
				break;

			case 0x58:  // X key
				{
					INT iAnswer = 0;

					_tprintf_s( _T( "\n\nDo you really want to shutdown the server? y/n\n" ) );
					iAnswer = _gettch();
					if( 'y' == iAnswer )
					{
						_tprintf_s( _T( "Prototype Server is shutting down. Please wait...\n" ) );
						SER()->SetbFinishServer( TRUE );
					}
					else
						_tprintf_s( _T( "Shutdown process has been canceled.\n" ) );
				}
				break;
			}
		}
	}
}

VOID DisplayCCU( VOID )
{
	DWORD dwCurtTick = timeGetTime();
	static DWORD dwDisplayCCUTick = 0;

	if( GetDeltaTime( dwDisplayCCUTick, dwCurtTick ) <= 13000 )
		return;

	LONG lCCU = g_CCU;

	static LONG lPCCU = 0;
	static SYSTEMTIME stPCCUTime;

	static LONG lDailyPCCU = 0;
	static SYSTEMTIME stDailyPCCUTime;

	TCHAR szConsoleTitle[MAX_PATH] = {0,};

	if( lPCCU < lCCU )
	{
		lPCCU = lCCU;
		GetLocalTime( &stPCCUTime );
	}

	if( lDailyPCCU < lCCU )
	{
		lDailyPCCU = lCCU;
		GetLocalTime( &stDailyPCCUTime );
	}

	_stprintf_s( szConsoleTitle, _countof( szConsoleTitle ),
				 _T( "Prototype Server - PCCU:%4d(%02d.%02d:%02d) DailyPCCU:%4d(%02d.%02d:%02d) CCU:%4d" ),
				 lPCCU, stPCCUTime.wDay, stPCCUTime.wHour, stPCCUTime.wMinute,
				 lDailyPCCU, stDailyPCCUTime.wDay, stDailyPCCUTime.wHour, stDailyPCCUTime.wMinute,
				 lCCU );
	SetConsoleTitle( szConsoleTitle );

	SYSTEMTIME stCurtTime;
	GetLocalTime( &stCurtTime );
	if( stCurtTime.wDay != stDailyPCCUTime.wDay )
	{
		TLOG( LOG_FILE,
			  _T( "[INFO] Prototype Server - PCCU:%4d(%02d. %02d:%02d) DailyPCCU:%4d(%02d. %02d:%02d) CCU:%4d" ),
			  lPCCU, stPCCUTime.wDay, stPCCUTime.wHour, stPCCUTime.wMinute,
			  lDailyPCCU, stDailyPCCUTime.wDay, stDailyPCCUTime.wHour, stDailyPCCUTime.wMinute,
			  lCCU );

		lDailyPCCU = 0;
		ZeroMemory( &stDailyPCCUTime, sizeof( stDailyPCCUTime ) );
	}

	dwDisplayCCUTick = dwCurtTick;
}
