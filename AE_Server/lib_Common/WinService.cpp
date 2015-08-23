//
// Win Service
//
// Original code by SYNC
// 2004.4 By CJC
//
//

//#include <TChar.h>
//#include <stdio.h>
//#include <string>

#include "CommonPCH.h"

WinService::WinService() : handleServiceEvent( INVALID_HANDLE_VALUE )
{
}

WinService::~WinService()
{
}

void WinService::initialize()
{
}

//
//void WinService::addEventLog( String serverName, LPCTSTR format, ... )
//{
//	String		temp;
//	TCHAR buffer[1024];
//	va_list args;
//
//	va_start( args, format );
//	wvsprintf( buffer, format, args );
//	va_end( args );
//
//	temp = buffer;
//	addEventLog( serverName, temp );
//}
//
//void WinService::addEventLog( String serverName, String log, EventLogType type )
//{
//    HANDLE		hEventSource;
//	String		temp;
//
//    hEventSource = RegisterEventSource( 0, serverName.getBuffer() );
//
//	temp = _T( "\n" ) + log + _T( "\nGetLastError() returned =" ) + (int)GetLastError();
//
//    if( hEventSource != 0 )
//	{
//		LPCTSTR string = temp.getBuffer();
//
//        ReportEvent( hEventSource, type, 0, 0, 0, 1, 0, &string, 0 );
//
//        DeregisterEventSource( hEventSource );
//    }
//}
//


bool WinService::uploadService( wchar_t* svcName, wchar_t* displayName, wchar_t* modulePath )
{
	SC_HANDLE handleManager;
	SC_HANDLE handleService;


	handleManager = ::OpenSCManagerW( NULL, NULL, SC_MANAGER_CREATE_SERVICE );
	if ( handleManager == 0 )
		return false;


	handleService = ::CreateServiceW( handleManager,
									  svcName,
									  displayName,
									  0,							// SERVICE_QUERY_STATUS
									  SERVICE_WIN32_OWN_PROCESS,
									  SERVICE_DEMAND_START,
									  SERVICE_ERROR_IGNORE,			// SERVICE_ERROR_NORMAL
									  modulePath,
									  NULL,
									  NULL,
									  NULL,
									  NULL,
									  NULL );
	if ( handleService == 0 )
	{
		::CloseServiceHandle( handleManager );
		return false;
	}

	::CloseServiceHandle( handleService );
	::CloseServiceHandle( handleManager );

	return true;
}

bool WinService::removeService( wchar_t* svcName )
{
	SC_HANDLE handleManager;
	SC_HANDLE handleService;

	handleManager = ::OpenSCManagerW( NULL, NULL, SC_MANAGER_CREATE_SERVICE );
	if( handleManager == 0 ) return false;

	handleService = ::OpenServiceW( handleManager, svcName, DELETE );

	// 서비스 종료

	::DeleteService( handleService );

	::CloseServiceHandle( handleService );
	::CloseServiceHandle( handleManager );

	return true;
}

bool WinService::startDispatcher( wchar_t* serviceName, bool ( *startCallBack )(), void ( *stopCallBack )() )
{
	serviceName = serviceName;

	SERVICE_TABLE_ENTRYW dispatch[] =
	{
		{ serviceName, serviceLoop },
		{ NULL, NULL }
	};


	serviceStartFunc = startCallBack;
	serviceStopFunc  = stopCallBack;

	if( ::StartServiceCtrlDispatcherW( dispatch ) == 0 )
	{
		DWORD Error = GetLastError();
		return false;
	}

	return true;
}

bool WinService::onServiceStart()
{
	SERVICE_STATUS ss;

	handleServiceEvent  = ::CreateEvent( 0, 0, 0, 0 );
	handleServiceStatus = RegisterServiceCtrlHandlerW( serviceName.c_str(), serviceHandler );

	ss.dwServiceType		= SERVICE_WIN32_OWN_PROCESS;
	ss.dwCurrentState		= SERVICE_START_PENDING;
	ss.dwControlsAccepted	= SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	ss.dwWin32ExitCode		= NO_ERROR;
	ss.dwServiceSpecificExitCode = 0;
	ss.dwCheckPoint			= 1;
	ss.dwWaitHint			= 1000;

	::SetServiceStatus( handleServiceStatus, &ss );

	if( serviceStartFunc != 0 )
	{
		if( ( *serviceStartFunc )() == false )
			return false;
	}

	ss.dwCurrentState = SERVICE_RUNNING;
	ss.dwCheckPoint	  = 0;
	ss.dwWaitHint	  = 0;

	::SetServiceStatus( handleServiceStatus, &ss );

	return true;
}

void __stdcall WinService::serviceHandler( DWORD status )
{
	Instance()->statusEvent = status;
	::SetEvent( Instance()->handleServiceEvent );
}

void __stdcall WinService::serviceLoop( DWORD argc, LPTSTR *argv )
{
	bool loop = true;
	DWORD serviceState = SERVICE_RUNNING;

	if( Instance()->onServiceStart()  == false )
	{
		Instance()->onServiceStop();

		return;
	}

	while( loop )
	{
		::WaitForSingleObject( Instance()->handleServiceEvent, INFINITE );

		switch( Instance()->statusEvent )
		{
		case  SERVICE_CONTROL_STOP :
			serviceState = SERVICE_STOP_PENDING;
			loop = false;
			break;

		case  SERVICE_CONTROL_CONTINUE :
			serviceState = SERVICE_RUNNING;
			break;

		case  SERVICE_CONTROL_PAUSE :
			serviceState = SERVICE_PAUSED;
			break;

		case  SERVICE_CONTROL_INTERROGATE :
			break;

		case  SERVICE_CONTROL_SHUTDOWN :
			serviceState = SERVICE_STOP_PENDING;
			loop = false;
			break;

		default :
			break;
		}
	}

	Instance()->onServiceStop();
}

void WinService::onServiceStop()
{
	SERVICE_STATUS ss;

	ss.dwServiceType		= SERVICE_WIN32_OWN_PROCESS;
	ss.dwControlsAccepted	= SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	ss.dwWin32ExitCode		= NO_ERROR;
	ss.dwServiceSpecificExitCode	= 0;
	ss.dwCurrentState		= SERVICE_STOP_PENDING;
	ss.dwCheckPoint			= 1;
	ss.dwWaitHint			= 5000;
	::SetServiceStatus( handleServiceStatus, &ss );

	( *serviceStopFunc )();

	ss.dwCurrentState		= SERVICE_STOPPED;
	ss.dwCheckPoint			= 0;
	ss.dwWaitHint			= 0;
	::SetServiceStatus( handleServiceStatus, &ss );

	if( handleServiceEvent == INVALID_HANDLE_VALUE )
	{
		::CloseHandle( handleServiceEvent );
		handleServiceEvent = INVALID_HANDLE_VALUE;
	}
}

//BOOL WinService::queryService( DWORD* pResult, LPCTSTR netMachine, LPCTSTR serviceName )
//{
//	SC_HANDLE handleService, handleServerConnect;
//
//	handleService = ::OpenSCManager( netMachine, 0, GENERIC_READ );
//	if( handleService == 0 )
//	{
//		*pResult = ::GetLastError();
//		return FALSE;
//	}
//
//	handleServerConnect = ::OpenService( handleService, serviceName, SERVICE_INTERROGATE );
//	if( handleServerConnect == 0 )
//	{
//		*pResult = ::GetLastError();
//		::CloseServiceHandle( handleService );
//		return FALSE;
//	}
//
//	do
//	{
//		if( !::ControlService( handleServerConnect, SERVICE_CONTROL_INTERROGATE, &ssQueryControl ) )
//			*pResult = ::GetLastError();
//	} while( ssQueryControl.dwCurrentState != SERVICE_STOPPED &&
//			ssQueryControl.dwCurrentState != SERVICE_RUNNING &&
//			ssQueryControl.dwCurrentState != SERVICE_PAUSED &&
//			ssQueryControl.dwCurrentState != 1 && handleServerConnect != 0 );
//
//	switch( ssQueryControl.dwCurrentState )
//	{
//	case SERVICE_STOPPED:
//	case SERVICE_RUNNING:
//	case SERVICE_PAUSED:
//		break;
//	default: 
//		ssQueryControl.dwCurrentState = 0;
//		break;
//	}
//
//	::CloseServiceHandle( handleServerConnect );
//	::CloseServiceHandle( handleService );
//
//	*pResult = ssQueryControl.dwCurrentState;
//	return TRUE;
//}
//
BOOL WinService::queryServiceStatus( DWORD* pResult, wchar_t* netMachine, wchar_t* serviceName )
{
	SC_HANDLE handleService, handleServerConnect;

	handleService = ::OpenSCManagerW( netMachine, 0, GENERIC_READ );
	if( handleService == 0 )
	{
		*pResult = ::GetLastError();
		return FALSE;
	}

	handleServerConnect = ::OpenServiceW( handleService, serviceName, SERVICE_QUERY_STATUS );
	if( handleServerConnect == 0 )
	{
		*pResult = ::GetLastError();
		::CloseServiceHandle( handleService );
		return FALSE;
	}

	if( !QueryServiceStatus( handleServerConnect, &ssQueryControl ) )
	{
		*pResult = ::GetLastError();
		::CloseServiceHandle( handleServerConnect );
		::CloseServiceHandle( handleService );
		return FALSE;
	}

	::CloseServiceHandle( handleServerConnect );
	::CloseServiceHandle( handleService );
	*pResult = ssQueryControl.dwCurrentState;
	return TRUE;
}

//BOOL WinService::startService( DWORD* pResult, LPCTSTR netMachine, LPCTSTR serviceName )
//{
//	SC_HANDLE handleService, handleServerConnect;
//
//	handleService = ::OpenSCManager( netMachine, 0, SC_MANAGER_CONNECT );
//	if( handleService == 0 )
//	{
//		*pResult = ::GetLastError();
//		return FALSE;
//	}
//
//	handleServerConnect = ::OpenService( handleService, serviceName, SERVICE_START | SERVICE_QUERY_STATUS );
//	if( handleServerConnect == 0 )
//	{
//		*pResult = ::GetLastError();
//		::CloseServiceHandle( handleService );
//		return FALSE;
//	}
//
//	if(::StartService( handleServerConnect, 0, 0 ) == TRUE )
//	{
//		if( ::QueryServiceStatus( handleServerConnect, &ssQueryControl ) )
//			::Sleep( ssQueryControl.dwWaitHint );
//	}
//	else
//	{
//		*pResult = ::GetLastError();
//
//		// check already running
//		::QueryServiceStatus( handleServerConnect, &ssQueryControl );
//		if( ssQueryControl.dwCurrentState == SERVICE_RUNNING )
//		{
//			*pResult = ssQueryControl.dwCurrentState;
//			return TRUE;
//		}
//
//		::CloseServiceHandle( handleServerConnect );
//		::CloseServiceHandle( handleService );
//		return FALSE;
//	}
//
//	::CloseServiceHandle( handleServerConnect );
//	::CloseServiceHandle( handleService );
//
//	*pResult = ssQueryControl.dwCurrentState;
//	return TRUE;
//}
//
//BOOL WinService::stopService( DWORD* pResult, LPCTSTR netMachine, LPCTSTR serviceName )
//{
//	SC_HANDLE handleService, handleServerConnect;
//
//	handleService = ::OpenSCManager( netMachine, 0, SC_MANAGER_CONNECT );
//	if( handleService == 0 )
//	{
//		*pResult = ::GetLastError();
//		return FALSE;
//	}
//
//	handleServerConnect = ::OpenService( handleService , serviceName, GENERIC_EXECUTE | SERVICE_QUERY_STATUS );
//	if( handleServerConnect == 0 )
//	{
//		*pResult = ::GetLastError();
//		::CloseServiceHandle( handleService );
//		return FALSE;
//	}
//
//	if( ::ControlService( handleServerConnect, SERVICE_CONTROL_STOP, &ssQueryControl ) == TRUE )
//	{
//		if( ::QueryServiceStatus( handleServerConnect, &ssQueryControl ) )
//			::Sleep( ssQueryControl.dwWaitHint );
//	}
//	else
//	{
//		*pResult = ::GetLastError();
//		::CloseServiceHandle( handleServerConnect );
//		::CloseServiceHandle( handleService );
//
//		// check already stopped
//		if( ssQueryControl.dwCurrentState == SERVICE_STOPPED )
//		{
//			*pResult = ssQueryControl.dwCurrentState;
//			return TRUE;
//		}
//		return FALSE;
//	}
//
//	::CloseServiceHandle( handleServerConnect );
//	::CloseServiceHandle( handleService );
//
//	*pResult = ssQueryControl.dwCurrentState;
//	return TRUE;
//}
//
//bool WinService::connectNet( LPCTSTR account, LPCTSTR password, LPCTSTR netMachine )
//{
//	String temp( _T( "\\\\" ) );
//	NETRESOURCE netInfo;
//
//	temp += netMachine;
//
//	netInfo.dwScope			= RESOURCE_GLOBALNET;
//	netInfo.dwType			= RESOURCETYPE_ANY;
//	netInfo.dwDisplayType	= RESOURCEDISPLAYTYPE_DOMAIN;
//	netInfo.dwUsage			= RESOURCEUSAGE_CONNECTABLE;
//	netInfo.lpLocalName		= 0;
//	netInfo.lpRemoteName	= temp.getBuffer();
//	netInfo.lpComment		= 0;
//	netInfo.lpProvider		= 0;
//
//	if( ::WNetAddConnection2( &netInfo, password, account, CONNECT_UPDATE_PROFILE ) == NO_ERROR )
//		return true;
//
//	return false;
//}