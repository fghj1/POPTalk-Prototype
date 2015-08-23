//
// Win Service
//
// Original code by SYNC
// 2004.4 By CJC
//
//

#ifndef  __WINSERVICE_H__
#define  __WINSERVICE_H__
#include <WinSvc.h>
//#include "../Sqlite/TSingleton.h"
#include "./TSingleton.h"

class WinService : public TSingleton< WinService >
{
public:
	enum EventLogType
	{
		EVENTLOGTYPE_ERROR			= EVENTLOG_ERROR_TYPE,
		EVENTLOGTYPE_WARNING		= EVENTLOG_WARNING_TYPE,
		EVENTLOGTYPE_INFORMATION	= EVENTLOG_INFORMATION_TYPE
	};

public:
			WinService();
	virtual ~WinService();

	virtual	void				initialize();

//	static	void				addEventLog( String serverName, LPCTSTR format, ... );
//	static	void				addEventLog( String serverName, String log, EventLogType type = EVENTLOGTYPE_WARNING );

			//String			getServiceName(){ return serviceName; }
			bool				uploadService( wchar_t* svcName, wchar_t* displayName, wchar_t* modulePath );
			bool				removeService( wchar_t* svcName );

			bool				startDispatcher( wchar_t* svrName, bool ( *startCallBack )(), void ( *stopCallBack )() );


			//BOOL				queryService( DWORD* pResult, LPCTSTR netMachine, LPCTSTR serviceName );
			BOOL				queryServiceStatus( DWORD* pResult, wchar_t* netMachine, wchar_t* serviceName );
			//BOOL				startService( DWORD* pResult, LPCTSTR netMachine, LPCTSTR serviceName );
			//BOOL				stopService( DWORD* pResult, LPCTSTR netMachine, LPCTSTR serviceName );
			//bool				connectNet( LPCTSTR account, LPCTSTR password, LPCTSTR netMachine );

private:
	static	void	__stdcall	serviceHandler( DWORD status );
	static	void	__stdcall	serviceLoop( DWORD argc, LPTSTR *argv );
			bool				onServiceStart();
			void				onServiceStop();

			bool				( *serviceStartFunc )	();
			void				( *serviceStopFunc )	();

			std::wstring			serviceName;
			SERVICE_STATUS_HANDLE	handleServiceStatus;

			HANDLE					handleServiceEvent;
			DWORD					statusEvent;
			SERVICE_STATUS			ssQueryControl;
};

#endif
