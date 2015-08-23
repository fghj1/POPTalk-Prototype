#pragma  once

#include <windows.h>

#ifndef _WIN32_WINNT  // 필요한 최소 플랫폼을 Windows Vista로 지정합니다.
#define _WIN32_WINNT 0x0600  // 다른 버전의 Windows에 맞도록 적합한 값으로 변경해 주십시오.
#endif


class TCriticalSection : public CRITICAL_SECTION
{
public:
#if( _WIN32_WINNT <= 0x402 )
	TCriticalSection()
	{
		::InitializeCriticalSection( const_cast<TCriticalSection*>( this ) );
	}
#endif

	virtual ~TCriticalSection()
	{
		::DeleteCriticalSection( const_cast<TCriticalSection*>( this ) );
	}

public:
#if( _WIN32_WINNT >= 0x403 )
	TCriticalSection( DWORD dwSpinCount = 4000 )
	{
		::InitializeCriticalSectionAndSpinCount( const_cast<TCriticalSection*>( this ), dwSpinCount );
	}

	DWORD SetSpinCount( DWORD dwSpinCount )
	{
		return ::SetCriticalSectionSpinCount( const_cast<TCriticalSection*>( this ), dwSpinCount );
	}
#endif

public:
	void lock() const
	{
		::EnterCriticalSection( const_cast<TCriticalSection*>( this ) );
	}

	void unlock() const
	{
		::LeaveCriticalSection( const_cast<TCriticalSection*>( this ) );
	}

	BOOL tryEnter()
	{
#if( _WIN32_WINNT >= 0x403 )
		return ::TryEnterCriticalSection( const_cast<TCriticalSection*>( this ) );
#else
		return FALSE;
#endif
	}
};

class CAutoLock
{
public:
	CAutoLock( TCriticalSection& rCS ) : m_rCS( rCS )
	{
		m_rCS.lock();
	}

	~CAutoLock( VOID )
	{
		m_rCS.unlock();
	}

	operator bool()
	{
		return false;
	}

private:
	TCriticalSection& m_rCS;
};

#define SCOPED_LOCK( A ) if( CAutoLock _syncVar = A ) { assert( 0 ); throw 0; } else
