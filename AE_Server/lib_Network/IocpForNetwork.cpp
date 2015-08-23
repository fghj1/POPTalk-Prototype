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

						// NOTE: ERROR_SEM_TIMEOUT �߻� �ñ�
						//		 A�� B�� ����Ǿ� ���� ��, A�� ���Ḧ �����ϸ� A�� shutdown( [socket], SD_SEND )�� ȣ���Ͽ� B���� half-close ��ȣ��
						//		 ������. B�� �� ��ȣ�� �����Ͽ� ������ ó�� ��, A���� A�� SD_SEND ��ȣ�� ���� ȸ������ B�� SD_SEND ��ȣ�� �������� ������
						//		 A�� �� 120�� �� A���� B�� ���õ� ������ ������� ERROR_SEM_TIMEOUT ������ ��ȯ�Ѵ�.

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
				return;  // CIocp::End()�� PostQueuedCompletionStatus( m_hIOCP, 0, 0, NULL );�� ���� �۾� ������ ����

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
