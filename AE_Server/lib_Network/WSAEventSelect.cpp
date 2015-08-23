#include "NetworkPCH.h"
#include "./WSAEventSelect.h"


CWSAEventSelect::CWSAEventSelect():m_hStartupEvent( NULL ), m_hSelectEvent( NULL ), m_hDestroyEvent( NULL ),
								   m_hWSAESThread( NULL ),
								   m_uni64WSAESSocket( NULL )
{
}

CWSAEventSelect::~CWSAEventSelect()
{
}

bool CWSAEventSelect::WSAESBegin( SOCKET uni64Socket )
{
	int iErrNo = 0;
	DWORD dwErrNo = 0;

	if( uni64Socket == NULL || m_uni64WSAESSocket != NULL )
		return false;

	m_uni64WSAESSocket = uni64Socket;

	m_hSelectEvent = WSACreateEvent();
	if( m_hSelectEvent == WSA_INVALID_EVENT )
	{
		iErrNo = WSAGetLastError();
		TLOG( LOG_DEBUG, _T( "[ERROR] ErrNo.%d" ), iErrNo );
		return false;
	}

	m_hDestroyEvent = CreateEvent( 0, FALSE, FALSE, 0 );
	if( m_hDestroyEvent == NULL )
	{
		dwErrNo = GetLastError();
		TLOG( LOG_DEBUG, _T( "[ERROR] ErrNo.%d" ), dwErrNo );
		return false;
	}

	m_hStartupEvent = CreateEvent( 0, FALSE, FALSE, 0 );
	if( m_hStartupEvent == NULL )
	{
		dwErrNo = GetLastError();
		TLOG( LOG_DEBUG, _T( "[ERROR] ErrNo.%d" ), dwErrNo );
		return false;
	}

	iErrNo = WSAEventSelect( uni64Socket, m_hSelectEvent, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE );
	if( iErrNo == SOCKET_ERROR )
	{
		iErrNo = WSAGetLastError();
		TLOG( LOG_DEBUG, _T( "[ERROR] ErrNo.%d" ), iErrNo );
		return false;
	}

	LPCTSTR lpszThreadName = _T( "WSAEventSelect Model" );
	m_hWSAESThread = CreateThreadEx( this, &CWSAEventSelect::WSAESThreadCallback, lpszThreadName );
	if( m_hWSAESThread == NULL )
	{ 
		dwErrNo = GetLastError();
		TLOG( LOG_DEBUG, _T( "[ERROR] ErrNo.%d" ), dwErrNo );
		return false;
	}

	WaitForSingleObject( m_hStartupEvent, INFINITE );

	return true;
}

VOID CWSAEventSelect::WSAESEnd( VOID )
{
	if( m_hWSAESThread )
	{
		SetEvent( m_hDestroyEvent );
		WaitForSingleObject( m_hWSAESThread, INFINITE );
		CloseHandle( m_hWSAESThread );
		m_hWSAESThread = NULL;
	}

	if( m_hStartupEvent )
	{
		CloseHandle( m_hStartupEvent );
		m_hStartupEvent = NULL;
	}

	if( m_hDestroyEvent )
	{
		CloseHandle( m_hDestroyEvent );
		m_hDestroyEvent = NULL;
	}

	if( m_hSelectEvent )
	{
		CloseHandle( m_hSelectEvent );
		m_hSelectEvent = NULL;
	}
}

// TODO: IOCP�� �ٲٴ� ���� ���?
// TODO: ȸ�� ��Ŷ ������ �ٲ� ���ɼ��� �ִ�.
//		 ���Ű� �۽� ó���� WSASelectEvent �� ������ �ϳ����� ���������
//		 ���ŵ� ��Ŷ�� ���� ó�� �� ȸ�� ��Ŷ ������ ������ �۾� �����忡�� ����ϱ� �����̴�.
//		 ������ ������� �Ǿ��ٰ� �ص� ���� ť�� ��� ��Ŷ�� �����Ͽ� ó���ϴ� ���� ���� �����忡��
//		 �ϱ� ������ ó�� �Ϸ�� �� ������ ���Ͽ� ���� �޶��� ���̸�, �̿� ���� ȸ�� ��Ŷ ������
//		 �۽� ť�� �ִ� �͵� ���ŵ� ������ ���� ���� �� ������ ���Ͽ� ���� ��� ���̴�.
//		 �̿� ���� �۽� ť�� ��� ���� ������� �۽��ϴٰ� �ص� �̰��� ���ŵ� ������ �´� ���� �ƴϴ�.
//		 �۽� ������ ���ŵ� ������ �°� ������ �� �ִ� ����� ���� ������� �ε��� ��ȣ�� �ٿ� �� �ε��� ��ȣ�� ���� ��Ŷ�� �� ������
//		 ��ũ�� ����Ʈ�� ��� �۾� �����忡�� �̸� �����Ͽ� �ε��� ��ȣ�� �����ص� ���¿��� ��Ŷ�� ó���Ͽ� ȸ���ؾ��� ���,
//		 �۽� ť�� ���� ��, �����ص� �ε��� ��ȣ�� ���� �̷�� ��ũ�� ����Ʈ�� ��´�. �׸��� �̸� �ε��� ��ȣ�� �������� �����Ѵ�.
//		 �۽��ص� �Ǵ� ��Ȳ�� ���� ���ĵ� ��ũ�� ����Ʈ���� �ε��� ��ȣ ������ ȸ���ϴ� ���̴�. ���?
DWORD CWSAEventSelect::WSAESThreadCallback( LPCTSTR lpszThreadName )
{
	WSANETWORKEVENTS stNetworkEvents;
	DWORD dwEventID = 0;
	HANDLE hOperatingEvents[2] = { m_hDestroyEvent, m_hSelectEvent };
	INT iResult = 0;

	while( true )
	{
		SetEvent( m_hStartupEvent );

		dwEventID = WaitForMultipleObjects( 2, hOperatingEvents, FALSE, INFINITE );
		switch( dwEventID )
		{
		case WAIT_OBJECT_0:
			return dwEventID;

		case WAIT_OBJECT_0 + 1:
			iResult = WSAEnumNetworkEvents( m_uni64WSAESSocket, m_hSelectEvent, &stNetworkEvents );
			if( iResult == SOCKET_ERROR )
			{
				iResult = WSAGetLastError();
				if( iResult == WSAENOTSOCK )
					TLOG( LOG_DEBUG, _T( "[ERROR] Not connected. ErrNo.%d" ), iResult );
				else
					TLOG( LOG_DEBUG, _T( "[ERROR] Need to check. ErrNo.%d" ), iResult );
				// TODO: [�翬�� ó��]������ ������ ��쿡 ����Ͽ� ������ �翬���ϴ� ó���� �ʿ��ϴ�.
				//		 ���⼭ ��ȯ���� �ʾ� �� ���� �޽����� while�� ���� ��� ��µȴ�.
			}
			else
			{
				// ����! FD_CLOSE, FD_CONNECT, FD_WRITE, FD_READ ������ �˻� ������ ������ �ʴ� ���� �����ϸ�,
				//		 ������ �˻�� ���������� �̷����� ���� ����.
				if( stNetworkEvents.lNetworkEvents & FD_CLOSE )
				{
					if( stNetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0 )
						TLOG( LOG_DEBUG, _T( "[ERROR] FD_CLOSE failed with ErrNo.%d" ), stNetworkEvents.iErrorCode[FD_CLOSE_BIT] );

					// TODO: [�翬�� ó��]DWORD CWSAEventSelect::WSAESThreadCallback(LPCTSTR lpszThreadName)�� ���� ������ �翬���ϴ� ó���� �ʿ��ϴ�.
					//		 ���� ������ �˼� ���� ������ ������ ������ �ٽ� ����ǰų� �Ǵ� ��Ʈ�� ������ �Ͻ������� �������ٰ� �ٽ� ����ȴٸ�
					//		 �̷� ��Ȳ�� ����� �翬���ϴ� ó���� �ʿ��ϴ�.
					OnIODisconnect();
				}

				if( stNetworkEvents.lNetworkEvents & FD_CONNECT )
				{
					if( stNetworkEvents.iErrorCode[FD_CONNECT_BIT] != 0 )
					{
						TLOG( LOG_DEBUG, _T( "[ERROR] FD_CONNECT failed with ErrNo.%d" ), stNetworkEvents.iErrorCode[FD_CONNECT_BIT] );
						break;
					}

					OnIOConnect();
				}

				if( stNetworkEvents.lNetworkEvents & FD_WRITE )
				{
					if( stNetworkEvents.iErrorCode[FD_WRITE_BIT] != 0 )
					{
						TLOG( LOG_DEBUG, _T( "[ERROR] FD_WRITE failed with ErrNo.%d" ), stNetworkEvents.iErrorCode[FD_WRITE_BIT] );
						break;
					}

					// TODO: ���ڰ� �ִ� SendPacket()�� ���� SendPacket()�� �����Ͽ� ��� �۽�, ��ȯ �۽� ó���� �ϵ��� �ߴ�.
					//		 �̷��� �۽� ó���� �ϸ� ������ �����ϴ� ó���� ���� �����̱� ������ ���ŵ� ������� ȸ������ �ʰ� �۾� ������ ���Ͽ� ���� ȸ����
					//		 �̷�����. �̿� ���� ������ �ʿ��ϴ�.

					//		 �̷��� �۽� ó���� �̷����� ���ڰ� �ִ� ���� ȣ��Ǵ� �Ͱ� ���ڰ� ���� ���� ȣ��Ǵ� �Ϳ� ���� ������ �۽� ���¸� ������ �� �ִ�.
					//		 ���ڰ� �ִ� ���� �ַ� ȣ��ȴٴ� ���� ������ �۽� ���°� ���ٴ� ���� �ǹ��ϸ�, ���ÿ� ������ ���¿� ��Ʈ�� ���µ� ���ٰ� �� �� �ִ�.
					//		 ���ڰ� ���� ���� �ַ� ȣ��ȴٴ� ���� ������ �۽� ���°� �����ٴ� ���� �ǹ��ϸ�, �۽��ؾ� �ϴ� ��Ŷ�� �з� �ִٴ� ���̴�.
					//		 ���� ���ڰ� �ִ� �Ͱ� ���ڰ� ���� �Ϳ� ȣ�� ������ ����� ���� �����ڿ��� �����شٸ� ������ �۽� ���¸� ���������� �˷��ְ� �� ���̴�.

					SendPacket();
				}

				if( stNetworkEvents.lNetworkEvents & FD_READ )
				{
					if( stNetworkEvents.iErrorCode[FD_READ_BIT] != 0 )
					{
						TLOG( LOG_DEBUG, _T( "[ERROR] FD_READ failed with ErrNo.%d" ), stNetworkEvents.iErrorCode[FD_READ_BIT] );
						break;
					}

					ReceivePacket();
				}
			}
			break;

		default:
			TLOG( LOG_DEBUG, _T( "[ERROR] Unknown value(%d)." ), dwEventID );
			break;
		}  // switch( dwEventID )
	}  // while( true )

	return dwEventID;
}
