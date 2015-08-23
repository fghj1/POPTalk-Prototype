#include "NetworkPCH.h"
#include "IocpForThreadPool.h"


bool CIocpForThreadPool::RegisterSocketToIocp( SOCKET socket, ULONG_PTR completionKey )
{
	return ( ( CIocp::RegisterSocketToIocp( socket, completionKey ) != FALSE )? true:false );
}

BOOL CIocpForThreadPool::PostQueuedCompletionStatus( DWORD dwNumberOfBytesTransferred, ULONG_PTR dwCompletionKey, LPOVERLAPPED lpOverlapped )
{
	return CIocp::PostQueuedCompletionStatus( dwNumberOfBytesTransferred, dwCompletionKey, lpOverlapped );
}

VOID CIocpForThreadPool::WorkerThreadFunc( VOID )
{
	BOOL bResult = FALSE;
	DWORD dwNumberOfBytes = 0, dwMilliseconds = INFINITE, dwErrNo = 0;
	LPVOID pCompletionKey = NULL;
	LPOVERLAPPED pstOverlapped = NULL;

	OVERLAPPED_EX* pstOverlappedEx = NULL;
	PVOID pWork = NULL;

	SetEvent( m_hStartupEvent );

	while( TRUE )
	{
		bResult = GetQueuedCompletionStatus( m_hIOCP, &dwNumberOfBytes, reinterpret_cast<PULONG_PTR>( &pCompletionKey ), &pstOverlapped, dwMilliseconds );
		if( bResult == FALSE )
		{
			dwErrNo = GetLastError();
			COverlappedEXDestructor ToDestroy( pstOverlapped );
			if( dwErrNo == WAIT_TIMEOUT )
			{
				TLOG( LOG_DEBUG,
					  _T( "[WARNING] A completion packet does not appear within the specified time(%lu)." ),
					  dwMilliseconds );
			}
			else if( dwErrNo == ERROR_ABANDONED_WAIT_0 )
			{
				TLOG( LOG_DEBUG, _T( "[ERROR] The I/O completion port handle(%#p) is closed." ), m_hIOCP );
				StartShutdown();
				return;
			}
			else
			{
				if( pstOverlapped == NULL )
				{
					TLOG( LOG_DEBUG,
						  _T( "[WARNING] ErrNo.%d. Failed to extract completion packet from the I/O completion queue." ),
						  dwErrNo );
					TLOG( LOG_DEBUG,
						  _T( "          lpNumberOfBytes : %lu, lpCompletionKey : %#p, lpOverlapped : %#p" ),
						  dwNumberOfBytes, pCompletionKey, pstOverlapped );
				}
				else
				{
					pstOverlappedEx = ( OVERLAPPED_EX* )pstOverlapped;

					if( dwNumberOfBytes == 0 )
					{
						TLOG( LOG_DEBUG,
							  _T( "[WARNING] ErrNo : %lu, lpNumberOfBytes : %lu, lpCompletionKey : %#p, lpOverlapped : %#p, I/O operation : %d" ),
							  dwErrNo, dwNumberOfBytes, pCompletionKey, pstOverlapped, ( pstOverlappedEx->IOType ) );

						pWork = pstOverlappedEx->pParameter;
						if( pWork == NULL )
						{
							TLOG( LOG_DEBUG, _T( "[WARNING] Overlapped parameter(%#p, %#p) is invalid." ), pstOverlappedEx, pWork );
							continue;
						}

						if( dwErrNo == ERROR_OPERATION_ABORTED )
						{
							TLOG( LOG_DEBUG, _T( "[WARNING] The I/O operation has been aborted because of an application request." ) );
							continue;
						}

						continue;
					}
					else
					{
						TLOG( LOG_DEBUG,
							  _T( "[WARNING] ErrNo.%d. Dequeues a completion packet for a failed I/O operation from the I/O completion port." ),
							  dwErrNo );
						TLOG( LOG_DEBUG,
							  _T( "          lpNumberOfBytes : %lu, lpCompletionKey : %#p, lpOverlapped : %#p, I/O operation : %d" ),
							  dwNumberOfBytes, pCompletionKey, pstOverlapped, ( pstOverlappedEx->IOType ) );
					}
				}  // if( pstOverlapped == NULL ) else
			}  // else if( dwErrNo == ERROR_ABANDONED_WAIT_0 ) else
		}  // if( bResult == FALSE )
		else
		{
			if( pstOverlapped == NULL )
				return;  // CIocp::End()의 PostQueuedCompletionStatus( m_hIOCP, 0, 0, NULL );에 의한 작업 스레드 종료

			COverlappedEXDestructor ToDestroy( pstOverlapped );
			pstOverlappedEx = ( OVERLAPPED_EX* )pstOverlapped;

			if( IO_WORK != pstOverlappedEx->IOType )
			{
				TLOG( LOG_DEBUG, _T( "[ERROR] Unknown value(%d)." ), ( pstOverlappedEx->IOType ) );
				continue;
			}

			pWork = pstOverlappedEx->pParameter;
			if( pWork == NULL )
			{
				TLOG( LOG_DEBUG, _T( "[WARNING] Overlapped parameter(%#p, %#p) is invalid." ), pstOverlappedEx, pWork );
				continue;
			}

			WorkProcessing( pCompletionKey, pWork );
		}  // if( bResult == FALSE ) else
	}  // while( TRUE )
}

bool CIocpForThreadPool::Begin( VOID )
{
	if( CIocp::Begin() == FALSE )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to begin the IOCP model." ) );
		return false;
	}

	return true;
}

bool CIocpForThreadPool::End( VOID )
{
	if( CIocp::End() == FALSE )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to end the IOCP model." ) );
		return false;
	}

	return true;
}
