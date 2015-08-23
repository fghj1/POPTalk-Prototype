#include "CorePCH.h"
#include <Windows.h>
#include <time.h>
#include <stdio.h>
#include  "TLog.h"
#include <DbgHelp.h>

#pragma comment( lib, "Dbghelp.lib" )

LONG WINAPI UnhandledExceptionCallStack( PEXCEPTION_POINTERS pExceptionInfo )
{
	MINIDUMP_EXCEPTION_INFORMATION MinidumpExceptionInformation;

	MinidumpExceptionInformation.ThreadId = ::GetCurrentThreadId();
	MinidumpExceptionInformation.ExceptionPointers = pExceptionInfo;
	MinidumpExceptionInformation.ClientPointers = FALSE;

	TCHAR* moduleName = LOGF()->getDumpFilename();
	if (moduleName == NULL)
	{
		::TerminateProcess(::GetCurrentProcess(), 0);
	}

	HANDLE hDumpFile = ::CreateFile(moduleName,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	MiniDumpWriteDump(GetCurrentProcess(),
		GetCurrentProcessId(),
		hDumpFile,
		MiniDumpNormal,
		&MinidumpExceptionInformation,
		NULL, NULL);

	::TerminateProcess(::GetCurrentProcess(), 0);

	return 0;
}

TLogFile::TLogFile() : m_hStdOutput(0), m_oldday(0), m_oldhour(0), m_type(LOG_CRT)
{
	setLogDirectory();

	m_hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	m_type = LOG_FILE;

	ZeroMemory(&m_QPFrequency, sizeof(m_QPFrequency));
	ZeroMemory(&m_ElapsedTime, sizeof(m_ElapsedTime));
	ZeroMemory(&m_StartCounter,sizeof(m_StartCounter));
	m_Retval = 0;

	// Query Frecuency
	m_Retval = QueryPerformanceFrequency(&m_QPFrequency);

	m_appName = NULL;
}

TLogFile::~TLogFile()
{
	m_file.Close();
}

bool TLogFile::setLogDirectory( VOID )
{
	HMODULE hModule;
	TCHAR szTemp[MAX_PATH] = {0,};
	sCHAR strDirectoryPath;
	bool bResult = false;
	DWORD dwResult = 0;
	INT nIndex = 0;

	do
	{
		// 실행중인 Server process의 Handle
		hModule = GetModuleHandle( NULL );
		if( hModule == NULL )
		{
			_stprintf_s( szTemp, _countof( szTemp ), _T( "[ERROR] Failed to extract the module handle.\r\n" ) );
			break;
		}

		// 실행된 Server의 경로 추출
		dwResult = GetModuleFileName( hModule, szTemp, MAX_PATH );
		if( dwResult == 0 )
		{
			_stprintf_s( szTemp, _countof( szTemp ), _T( "[ERROR] Failed to extract the module name.\r\n" ) );
			break;
		}
		strDirectoryPath = szTemp;

		// 로그 기록 경로 지정
		INT nDepth = 0;
		bResult = true;
		for( nDepth = -2; ( nDepth < 0 && true == bResult ); ++nDepth )
		{
			nIndex = strDirectoryPath.findLast( _T( '\\' ) );
			if( nIndex == -1 )
			{
				nIndex = strDirectoryPath.findLast( _T( '/' ) );
				if( nIndex == -1 )
				{
					_stprintf_s( szTemp, _countof( szTemp ), _T( "[ERROR] Failed to find the '/'.\r\n" ) );
					bResult = false;
				}
			}

			strDirectoryPath.replace( ( U32 )nIndex, _T( '\0' ) );
		}
		if( false == bResult )
			break;
		else
			strDirectoryPath += _T( '\\' );

#ifdef WIN64
		strDirectoryPath += _T( "Log\\x64\\" );
#else
		strDirectoryPath += _T( "Log\\x86\\" );
#endif
		m_LogDirectory = strDirectoryPath;

		// 경로 생성
		TCHAR szDirectoryName[MAX_PATH] = {0,};
		TCHAR* pPathPointer = strDirectoryPath.c_str();
		TCHAR* pNamePointer = szDirectoryName;

		while( *pPathPointer )
		{
			if( ( _T( '\\' ) == *pPathPointer ) || ( _T( '/' ) == *pPathPointer ) )
			{
				if( _T( ':' ) != *( pPathPointer - 1 ) )
					CreateDirectory( szDirectoryName, NULL );
			}

			*pNamePointer++ = *pPathPointer++;
			*pNamePointer = _T( '\0' );
		}

		if( FALSE == CreateDirectory( strDirectoryPath.c_str(), NULL ) )
		{
			dwResult = GetLastError();
			if( ERROR_ALREADY_EXISTS == dwResult )
				bResult = true;
			else
				_stprintf_s( szTemp, _countof( szTemp ), _T( "[WARNING] Failed to create a 'Log' directory.\r\n" ) );
		}
	}
	while( false );

	if( false == bResult )
	{
		OutputDebugString( szTemp );
		_tprintf_s( szTemp );
		strDirectoryPath.Empty();
	}

	return bResult;
}

//------------------------------------------------------------------------------
// starts profiling
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// ProfileStart
//------------------------------------------------------------------------------
void TLogFile::ProfileStart()
{
	m_Retval = QueryPerformanceCounter (&m_StartCounter);
}

//------------------------------------------------------------------------------
// ProfileEnd 
//------------------------------------------------------------------------------
__int64 TLogFile::ProfileEnd(LPCTSTR szFuncname)
{

	m_Retval = QueryPerformanceCounter (&m_EndCounter);
	m_ElapsedTime = (m_EndCounter.QuadPart  - m_StartCounter.QuadPart );

	CRT(_T("%s time: %I64d ticks ( %.3fsecs )"), szFuncname, m_ElapsedTime, SecsFromTicks(m_ElapsedTime) );
	return m_ElapsedTime;
}


void TLogFile::CRT(TCHAR* fmt, ...)
{
	m_Lock.lock();

	TCHAR szLog[1024] = _T( "" );
	va_list args;
	va_start( args, fmt );
	tVSPRINTF_S( szLog, 1024, fmt, args );

	szMessage.Empty();
	szMessage.format( _T( "%s\r\n" ), szLog );

	if( NULL != m_hStdOutput && INVALID_HANDLE_VALUE != m_hStdOutput )
		tPRINTF( _T( "%s" ), szMessage.c_str() );
	else
		Filelog( szMessage );

	va_end( args );

	m_Lock.unlock();
}

void TLogFile::DisplayEx(TCHAR* fmt, ...)
{
	m_Lock.lock();

	TCHAR szLog[1024] = _T("");
	va_list args;
	va_start(args,fmt);
	tVSPRINTF(szLog, 1024, fmt, args);

	szMessage.Empty();
	szMessage.format(_T("%s%s"), szLog,_T("\r\n"));

	if( NULL != m_hStdOutput && INVALID_HANDLE_VALUE != m_hStdOutput )
		tPRINTF(_T("%s"), szMessage.c_str());
	OutputDebugString( szMessage );
	Filelog( szMessage );

	va_end(args);

	m_Lock.unlock();
}

void TLogFile::Display(LPCTSTR szFuncname, TCHAR* fmt, ...)
{
	m_Lock.lock();

	TCHAR szLog[1024] = _T("");
	va_list args;
	va_start(args,fmt);
	tVSPRINTF(szLog, 1024, fmt, args);

	szMessage.Empty();
	szMessage.format(_T("[%7d][%s()]--> %s%s"), GetCurrentThreadId(), szFuncname, szLog, _T("\r\n"));

	if( NULL != m_hStdOutput && INVALID_HANDLE_VALUE != m_hStdOutput )
		tPRINTF(_T("%s"), szMessage.c_str());
	OutputDebugString( szMessage );
	Filelog( szMessage );

	va_end(args);

	m_Lock.unlock();
}

void TLogFile::DisplayCS(LPCTSTR szPosition, TCHAR* pLogMessage)
{
	m_Lock.lock();

	szMessage.Empty();
	szMessage.format(_T("[%7d][%s]--> %s\r\n"), GetCurrentThreadId(), szPosition, pLogMessage);

	if( NULL != m_hStdOutput && INVALID_HANDLE_VALUE != m_hStdOutput )
		tPRINTF(_T("%s"), szMessage.c_str());
	OutputDebugString( szMessage );
	Filelog( szMessage );

	m_Lock.unlock();
}

void TLogFile::WriteLogFile( LPCTSTR szFuncname, TCHAR* fmt, ... )
{
	m_Lock.lock();

	TCHAR szLog[1024] = _T( "" );
	va_list args;

	va_start( args,fmt );
	tVSPRINTF( szLog, 1024, fmt, args );

	szMessage.Empty();
	szMessage.format( _T( "[%s()]--> %s%s" ), szFuncname, szLog, _T( "\r\n" ) );

	Filelog( szMessage );

	va_end( args );

	m_Lock.unlock();
}

bool TLogFile::Filelog(LPCTSTR buf)
{
	TCHAR	temp_file_name[1024]	=_T("");	
	TCHAR	temp_date_buf[1024]		=_T("");
	TCHAR	temp_write_data[1024]	=_T("");	// 파일에 쓰는 날짜이기 때문에 유니코드
	sCHAR szLogFullPath;

	errno_t err;

	struct tm sTM;
	time_t	raw;		
	
	time(&raw);
	err = localtime_s(&sTM, &raw);	
	
	tSTRFTIME(temp_write_data, 1024, _T("%H:%M:%S : "), &sTM);	
	//strftime(temp_write_data, 1024, "%H:%M:%S : ", &sTM);	

	switch( m_type )
	{
		// Todo : 이 아래는 로그파일 남기는 단위는 하나로 통일해 버리는게 나을듯 하다. 굳이 여러 단계로 나눠놓을 필요는 없을 듯... iskim (reconfirm)
		// 기존 방식으로는 프로세스가 log 파일을 항상 점유하고 있어서 실시간으로 파일을 확인할 수 없었다. 프로세스 점유 시간을 줄이도록 수정함. iskim
	case LOG_FILE :		// 실시간
		{
			tSTRFTIME(temp_date_buf, 1024, _T("%Y-%m-%d-%H"), &sTM);
			tSPRINTF_S(temp_file_name, _T("%s_%s_pid%d.log"), getAppName(), temp_date_buf, m_pid);
		}
		break;
	case LOG_FILE_HOUR :			// 한시간에 파일 한개씩
		{
			tSTRFTIME(temp_date_buf, 1024, _T("%Y-%m-%d-%H"), &sTM);
			tSPRINTF_S(temp_file_name,  _T("%s_%s_pid%d.log"), getAppName(), temp_date_buf, m_pid);


			if( m_oldday != sTM.tm_yday || m_oldhour != sTM.tm_hour || (!m_file.isOpen()) )
			{				
				m_oldhour = sTM.tm_hour;
				m_oldday = sTM.tm_yday;
			} // if( m_oldtime < tm_ptr->tm_hour ||   == NULL ) 
		}
		break;


	case LOG_FILE_DAY :			// 하루에 파일 한개씩
		{
			tSTRFTIME(temp_date_buf, 1024, _T("%Y-%m-%d"), &sTM);
			tSPRINTF_S(temp_file_name,  _T("%s_%s_pid%d.log"), getAppName(), temp_date_buf, m_pid);


			if( m_oldday != sTM.tm_yday || (!m_file.isOpen()))
			{				
				m_oldday = sTM.tm_yday;
			} 
		}
		break;
	}

	szLogFullPath += m_LogDirectory + temp_file_name;
	if (m_file.Open(szLogFullPath.c_str(), _T("at")))
	{
		aCHAR timestamp = W2M(temp_write_data);
		aCHAR message = W2M(buf);
		m_file.Write(timestamp, timestamp.size(), 1 );
		m_file.Write(message, message.size(), 1 );
		m_file.Close();
	}

	return true;
}

void TLogFile::setApp( TCHAR* name, DWORD pid )
{
	m_appName = name;
	m_pid = pid;
}

TCHAR* TLogFile::getDumpFilename()
{
	TCHAR* temp_file_name = new TCHAR[1024];
	TCHAR temp_date_buf[1024] = L"";
	
	struct tm sTM;
	time_t raw;
	
	time(&raw);
	localtime_s(&sTM, &raw);

	tSTRFTIME(temp_date_buf, 1024, L"%Y-%m-%d-%H", &sTM);
	tSPRINTF_S(temp_file_name, 1024, L"crash_%s_%s_pid%d.dmp", getAppName(), temp_date_buf,  GetCurrentProcessId());

	return temp_file_name;
}

LPTOP_LEVEL_EXCEPTION_FILTER TLogFile::initMinidumpFilter()
{
	return SetUnhandledExceptionFilter(UnhandledExceptionCallStack);
}
