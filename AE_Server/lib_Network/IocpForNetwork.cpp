#include "NetworkPCH.h"
#include "IocpForNetwork.h"


bool CIocpForNetwork::RegisterSocketToIocp( SOCKET socket, ULONG_PTR completionKey )
{
	return ( ( CIocp::RegisterSocketToIocp( socket, completionKey ) != FALSE )? true:false );
}

VOID CIocpForNetwork::PostQueuedCompletionStatus( DWORD dwNumberOfBytesTransferred, ULONG_PTR dwCompletionKey, LPOVERLAPPED lpOverlapped )
{
	CIocp::PostQueuedCompletionStatus( dwNumberOfBytesTransferred, dwCompletionKey, lpOverlapped );
}

VOID CIocpForNetwork::WorkerThreadFunc( VOID )
{
	DWORD dwNumberOfBytes = 0, dwMilliseconds = INFINITE, dwErrNo = 0;
	LPVOID pCompletionKey = NULL;
	LPOVERLAPPED pstOverlapped = NULL;
	BOOL bResult = FALSE;

	OVERLAPPED_EX* pstOverlappedEx = NULL;
	PVOID pObject = NULL;

	srand( timeGetTime() );
	SetEvent( m_hStartupEvent );

	while( TRUE )
	{
		bResult = GetQueuedCompletionStatus( m_hIOCP, &dwNumberOfBytes, ( PULONG_PTR )&pCompletionKey, &pstOverlapped, dwMilliseconds );
		if( bResult == FALSE )
		{
			dwErrNo = GetLastError();
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
						if( pstOverlappedEx->IOType != IO_ACCEPT && pstOverlappedEx->IOType != IO_WRITE )
						{
							TLOG( LOG_DEBUG,
								  _T( "[WARNING] ErrNo.%d. The socket might have been removed from the I/O completion port." ),
								  dwErrNo );
							TLOG( LOG_DEBUG,
								  _T( "          lpNumberOfBytes : %lu, lpCompletionKey : %#p, lpOverlapped : %#p, I/O operation : %d" ),
								  dwNumberOfBytes, pCompletionKey, pstOverlapped, ( pstOverlappedEx->IOType ) );
						}

						pObject = pstOverlappedEx->pParameter;
						if( pObject == NULL )
						{
							TLOG( LOG_DEBUG, _T( "[WARNING] Overlapped parameter(%#p, %#p) is invalid." ), pstOverlappedEx, pObject );
							continue;
						}

						if( dwErrNo == ERROR_OPERATION_ABORTED )
						{
							TLOG( LOG_DEBUG, _T( "[WARNING] The I/O operation has been aborted because of an application request." ) );
							continue;
						}

						// NOTE: ERROR_SEM_TIMEOUT 발생 시기
						//		 A와 B가 연결되어 있을 때, A가 종료를 시작하면 A는 shutdown( [socket], SD_SEND )을 호출하여 B에게 half-close 신호를
						//		 보낸다. B는 이 신호에 대응하여 적절한 처리 후, A에게 A의 SD_SEND 신호에 대한 회신으로 B의 SD_SEND 신호를 보내주지 않으면
						//		 A는 약 120초 후 A내에 B와 관련된 소켓을 대상으로 ERROR_SEM_TIMEOUT 에러를 반환한다.

						OnIODisconnected( pObject );
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
				}

				if( pCompletionKey )
					OnIODisconnected( pCompletionKey, true );
			}  // else if( dwErrNo == ERROR_ABANDONED_WAIT_0 )
		}  // if( bResult == FALSE )
		else
		{
			if( pstOverlapped == NULL )
				return;  // CIocp::End()의 PostQueuedCompletionStatus( m_hIOCP, 0, 0, NULL );에 의한 작업 스레드 종료

			pstOverlappedEx = ( OVERLAPPED_EX* )pstOverlapped;
			pObject = pstOverlappedEx->pParameter;
			if( pObject == NULL )
			{
				TLOG( LOG_DEBUG, _T( "[WARNING] Overlapped parameter(%#p, %#p) is invalid." ), pstOverlappedEx, pObject );
				continue;
			}

			switch( pstOverlappedEx->IOType )
			{
			case IO_ACCEPT:
				OnIOConnected( pObject );
				break;

			case IO_READ:
				OnIORead( pObject, dwNumberOfBytes );
				break;

			case IO_WRITE:
				OnIOWrote( pObject );
				break;

			default:
				TLOG( LOG_DEBUG, _T( "[ERROR] Unknown value(%d)." ), ( pstOverlappedEx->IOType ) );
				break;
			}
		}  // if( bResult == FALSE ) else
	}  // while( TRUE )
}

bool CIocpForNetwork::Begin( VOID )
{
	if( CIocp::Begin() == FALSE )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to begin the IOCP model." ) );
		return false;
	}

	return true;
}

bool CIocpForNetwork::End( VOID )
{
	if( CIocp::End() == FALSE )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to end the IOCP model." ) );
		return false;
	}

	return true;
}
