#ifndef __SONOV_BERKANIX_LOG_HEADER__
#define __SONOV_BERKANIX_LOG_HEADER__


#include <tchar.h>
#include "../lib_Common/CommonPCH.h"
#include "./TFile.h"

const unsigned int MAX_BUF_SIZE = 2048;


typedef enum tagLogType
{
	LOG_CRT=1,
	LOG,
	LOG_DEBUG,
	LOG_FILE,
	PROFILE_START,
	PROFILE_END,
	//-----------------------------------
	LOG_FILE_HOUR,			// 한시간에 파일 한개씩
	LOG_FILE_DAY,			// 하루에 파일 한개씩
} LogType;

#define CONSOLE_BLUE   FOREGROUND_BLUE
#define CONSOLE_GREEN  FOREGROUND_GREEN
#define CONSOLE_RED    FOREGROUND_RED
#define CONSOLE_BRIGHT FOREGROUND_INTENSITY


enum eConsoleColor
{
	TTEM = 1,
	GREY    = CONSOLE_BLUE  | CONSOLE_GREEN | CONSOLE_RED,
	BLUE    = CONSOLE_BLUE  | CONSOLE_BRIGHT,
	GREEN   = CONSOLE_GREEN | CONSOLE_BRIGHT,
	RED     = CONSOLE_RED   | CONSOLE_BRIGHT,
	CYAN    = GREEN | BLUE,
	YELLOW  = RED   | GREEN,
	MAGENTA = RED   | BLUE,
	WHITE   = RED | GREEN | BLUE,
};

class TLogFile : public TSingleton< TLogFile >
{
	friend TSingleton<TLogFile>;

public:
	TLogFile();
	virtual ~TLogFile();	

private:	
	sCHAR			m_LogDirectory;
	HANDLE			m_hStdOutput;
	TFile			m_file;
	int				m_oldday;
	int				m_oldhour;
	int				m_type;

	bool setLogDirectory( VOID );

public:
	
	void CRT(TCHAR* fmt, ...);
	void DisplayEx(TCHAR* fmt, ...);
	void Display(LPCTSTR szFuncname, TCHAR* fmt, ...);
	void DisplayCS(LPCTSTR szPosition, TCHAR* pLogMessage); // C# 로그
	void WriteLogFile( LPCTSTR szFuncname, TCHAR* fmt, ... );
	bool Filelog( LPCTSTR buf );

	void setApp( TCHAR* name, DWORD pid);
	TCHAR* getAppName() { return m_appName; }
	TCHAR* getDumpFilename();
	LPTOP_LEVEL_EXCEPTION_FILTER initMinidumpFilter();

	//----------------Profile 
	void		ProfileStart(); 
	__int64		ProfileEnd(LPCTSTR szFuncname);
	
	// The following function convert ticks to seconds
	double SecsFromTicks (__int64 ticks)
	{
		return static_cast<double>(ticks) / static_cast<double>(m_QPFrequency.QuadPart);
	}

	inline DWORD GetLastRetVal()
	{
		return static_cast<DWORD>(m_Retval);
	}
private:
	TCriticalSection			m_Lock;
	sCHAR						szMessage;


	LARGE_INTEGER	m_QPFrequency;		// ticks/sec resolution
	LARGE_INTEGER	m_StartCounter;		// start time
	LARGE_INTEGER	m_EndCounter;		// finish time
	__int64			m_ElapsedTime;		// elapsed time
	DWORD			m_Retval;			// return value for API functions

	TCHAR*			m_appName;			// 실행중인 프로세스 이름
	DWORD			m_pid;				// 실행중인 프로세스ID
};


inline TLogFile* LOGF()
{
	return TLogFile::Instance();
}


//----------------------------------------------------------------
/// @def    TLOG
//----------------------------------------------------------------
#ifndef TKLOG_DISABLED
	
#define TLOG(LOG_TYPE, lpszMessage, ...) 	 \
		(LOG_TYPE == LOG_CRT  ? (LOGF()->CRT( lpszMessage, __VA_ARGS__ )) :	\
		(LOG_TYPE == LOG ? (LOGF()->DisplayEx(lpszMessage, __VA_ARGS__ )) :\
		(LOG_TYPE == LOG_DEBUG ?  (LOGF()->Display(_T(__FUNCTION__), lpszMessage, __VA_ARGS__ ))  :\
		(LOG_TYPE == LOG_FILE ?  (LOGF()->WriteLogFile(_T(__FUNCTION__), lpszMessage, __VA_ARGS__ ))  :\
		(LOG_TYPE == PROFILE_START ? (LOGF()->ProfileStart())  :\
		(LOG_TYPE == PROFILE_END ? (LOGF()->ProfileEnd(_T(__FUNCTION__)))  :\
		(LOGF()->DisplayEx( lpszMessage, __VA_ARGS__ )) ))))))
#else
	#define TLOG(LOG_TYPE, lpszMessage, ...) (lpszMessage)
#endif

class Timer : public TSingleton<Timer>
{
	friend TSingleton<Timer>;

public:
	Timer() 
	{
		QueryPerformanceFrequency(&m_QPFrequency);
	}
	~Timer() {}
private:
	LARGE_INTEGER	m_QPFrequency;		// ticks/sec resolution
	LARGE_INTEGER	m_StartCounter;		// start time
	LARGE_INTEGER	m_EndCounter;		// finish time
public:
	void Start()
	{
		QueryPerformanceCounter(&m_StartCounter);
	}
	__int64 End()
	{
		QueryPerformanceCounter(&m_EndCounter);
		return m_EndCounter.QuadPart - m_StartCounter.QuadPart;
	}

	__int64 GetCounter()
	{
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		return counter.QuadPart;
	}

	LARGE_INTEGER GetFrequency()
	{
		return m_QPFrequency;
	}
};

inline Timer* TIMER()
{
	return Timer::Instance();
}

#endif