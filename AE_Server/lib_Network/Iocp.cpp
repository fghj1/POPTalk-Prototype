#include "NetworkPCH.h"
#include "./Define_Network.h"
#include "Iocp.h"


DWORD WINAPI WorkerThreadCallback( LPVOID pParameter )
{
	CIocp* pOwner = ( CIocp* )pParameter;
	pOwner->WorkerThreadFunc();

	return 0;
}

CIocp::CIocp( VOID )
{
	m_hIOCP = NULL;
	m_dwWorkerThreadCount = 0;

	m_hStartupEvent = NULL;
}

BOOL CIocp::RegisterSocketToIocp( SOCKET socket, ULONG_PTR completionKey )
{
	if( socket == NULL || completionKey == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[WARNING] Parameter(%#p, %#p) is invalid." ), socket, completionKey );
		return FALSE;
	}

	m_hIOCP = CreateIoCompletionPort( ( HANDLE )socket, m_hIOCP, completionKey, 0 );
	if( m_hIOCP == NULL )
	{
		DWORD dwErrNo = GetLastError();
		TLOG( LOG_DEBUG, _T( "[ERROR] ErrNo.%d. Failed to associate the socket with IOCP." ), dwErrNo );
		return FALSE;
	}

	return TRUE;
}

BOOL CIocp::PostQueuedCompletionStatus( DWORD dwNumberOfBytesTransferred, ULONG_PTR dwCompletionKey, LPOVERLAPPED lpOverlapped )
{
	BOOL bResult = ::PostQueuedCompletionStatus( m_hIOCP, dwNumberOfBytesTransferred, dwCompletionKey, lpOverlapped );
	if( bResult == FALSE )
		TLOG( LOG_DEBUG, _T( "[ERROR] ErrNo.%d. Failed to post an I/O completion packet to an I/O completion port." ), GetLastError() );

	return bResult;
}

bool CIocp::Begin( VOID )
{
	SYSTEM_INFO SystemInfo;
	GetSystemInfo( &SystemInfo );

	m_dwWorkerThreadCount = SystemInfo.dwNumberOfProcessors;
	m_hIOCP = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, m_dwWorkerThreadCount );
	if( m_hIOCP == NULL )
		return false;

	m_hStartupEvent = CreateEvent( 0, FALSE, FALSE, 0 );
	if( m_hStartupEvent == NULL )
	{
		End();
		return false;
	}

	DWORD dwNum = 0;
	for( dwNum = 0; dwNum < m_dwWorkerThreadCount; ++dwNum )
	{
		HANDLE WorkerThread = CreateThread( NULL, 0, WorkerThreadCallback, this, 0, NULL );

		m_vectWorkerThreadHandle.push_back( WorkerThread );

		WaitForSingleObject( m_hStartupEvent, INFINITE );
	}

	return true;
}

BOOL CIocp::End( VOID )
{
	BOOL bResult = TRUE;
	DWORD dwNum = 0, dwResult = 0;
	HANDLE hWorkerThread = NULL;

	for( dwNum = 0; dwNum < m_vectWorkerThreadHandle.size(); ++dwNum )
	{
		bResult = ::PostQueuedCompletionStatus( m_hIOCP, 0, dwNum, NULL );
		if( bResult == FALSE )
			TLOG( LOG_DEBUG, _T( "[WARNING] ErrNo.%d. Failed to close a worker thread of IOCP." ), GetLastError() );
	}

	std::vector<HANDLE>::iterator itor;
	for( itor = m_vectWorkerThreadHandle.begin(); itor != m_vectWorkerThreadHandle.end(); )
	{
		hWorkerThread = *itor;
		if( hWorkerThread == NULL )
		{
			itor = m_vectWorkerThreadHandle.erase( itor );
			continue;
		}

		dwResult = WaitForSingleObject( hWorkerThread, 1979 );
		if( dwResult == WAIT_OBJECT_0 )
		{
			bResult = CloseHandle( hWorkerThread );
			if( bResult == FALSE )
				TLOG( LOG_DEBUG, _T( "[WARNING] ErrNo.%d. Failed to close an worker thread handle." ), GetLastError() );
			else
			{
				itor = m_vectWorkerThreadHandle.erase( itor );
				continue;
			}
		}
		else if( dwResult != WAIT_TIMEOUT )
			TLOG( LOG_DEBUG, _T( "[WARNING] ErrNo.%d. Failed to wait until the specified object is in the signaled state." ), GetLastError() );

		++itor;
	}

	if( m_vectWorkerThreadHandle.empty() == false )
		return FALSE;

	if( m_hIOCP )
	{
		bResult = CloseHandle( m_hIOCP );
		if( bResult == FALSE )
		{
			TLOG( LOG_DEBUG, _T( "[WARNING] ErrNo.%d. Failed to close an IOCP handle." ), GetLastError() );
			return FALSE;
		}
		else
			m_hIOCP = NULL;
	}

	if( m_hStartupEvent )
	{
		bResult = CloseHandle( m_hStartupEvent );
		if( bResult == FALSE )
		{
			TLOG( LOG_DEBUG, _T( "[WARNING] ErrNo.%d. Failed to close an event handle to start worker thread." ), GetLastError() );
			return FALSE;
		}
		else
			m_hStartupEvent = NULL;
	}

	return bResult;
}
