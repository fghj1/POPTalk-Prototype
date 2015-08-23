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

// TODO: IOCP로 바꾸는 것은 어떨까?
// TODO: 회신 패킷 순서가 바뀔 가능성이 있다.
//		 수신과 송신 처리는 WSASelectEvent 모델 쓰레드 하나에서 담당하지만
//		 수신된 패킷에 대한 처리 및 회신 패킷 구성은 여러개 작업 쓰레드에서 담당하기 때문이다.
//		 수신은 순서대로 되었다고 해도 수신 큐에 담긴 패킷을 추출하여 처리하는 것은 여러 쓰레드에서
//		 하기 때문에 처리 완료는 각 쓰레드 부하에 따라 달라질 것이며, 이에 맞춰 회신 패킷 구성후
//		 송신 큐에 넣는 것도 수신된 순서와 관계 없이 각 쓰레드 부하에 따라 담길 것이다.
//		 이에 따라 송신 큐에 담긴 것을 순서대로 송신하다고 해도 이것이 수신된 순서와 맞는 것은 아니다.
//		 송신 순서를 수신된 순서에 맞게 보장할 수 있는 방안은 수신 순서대로 인덱스 번호를 붙여 이 인덱스 번호와 수신 패킷을 한 쌍으로
//		 링크드 리스트에 담고 작업 쓰레드에서 이를 추출하여 인덱스 번호는 보유해둔 상태에서 패킷을 처리하여 회신해야할 경우,
//		 송신 큐에 담을 때, 보유해둔 인덱스 번호와 쌍을 이루어 링크드 리스트에 담는다. 그리고 이를 인덱스 번호를 기준으로 정렬한다.
//		 송신해도 되는 상황이 오면 정렬된 링크드 리스트에서 인덱스 번호 순서로 회신하는 것이다. 어떨까?
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
				// TODO: [재연결 처리]소켓을 해제한 경우에 대비하여 소켓을 재연결하는 처리가 필요하다.
				//		 여기서 반환되지 않아 위 에러 메시지가 while에 의해 계속 출력된다.
			}
			else
			{
				// 주의! FD_CLOSE, FD_CONNECT, FD_WRITE, FD_READ 각각에 검사 순서는 변하지 않는 것을 권장하며,
				//		 각각에 검사는 개별적으로 이뤄지는 것이 좋다.
				if( stNetworkEvents.lNetworkEvents & FD_CLOSE )
				{
					if( stNetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0 )
						TLOG( LOG_DEBUG, _T( "[ERROR] FD_CLOSE failed with ErrNo.%d" ), stNetworkEvents.iErrorCode[FD_CLOSE_BIT] );

					// TODO: [재연결 처리]DWORD CWSAEventSelect::WSAESThreadCallback(LPCTSTR lpszThreadName)에 게임 서버와 재연결하는 처리가 필요하다.
					//		 게임 서버가 알수 없는 이유로 비정상 종료후 다시 실행되거나 또는 네트웍 연결이 일시적으로 끊어졌다가 다시 연결된다면
					//		 이런 상황에 대비해 재연결하는 처리가 필요하다.
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

					// TODO: 인자가 있는 SendPacket()와 없는 SendPacket()로 구분하여 즉시 송신, 순환 송신 처리를 하도록 했다.
					//		 이렇게 송신 처리를 하면 순서를 보장하는 처리가 없는 상태이기 때문에 수신된 순서대로 회신하지 않고 작업 쓰레드 부하에 따라 회신이
					//		 이뤄진다. 이에 대한 개선이 필요하다.

					//		 이렇게 송신 처리가 이뤄지면 인자가 있는 것이 호출되는 것과 인자가 없는 것이 호출되는 것에 따라 서버의 송신 상태를 짐작할 수 있다.
					//		 인자가 있는 것이 주로 호출된다는 것은 서버의 송신 상태가 좋다는 것을 의미하며, 동시에 수신측 상태와 네트웍 상태도 좋다고 할 수 있다.
					//		 인자가 없는 것이 주로 호출된다는 것은 서버의 송신 상태가 안좋다는 것을 의미하며, 송신해야 하는 패킷이 밀려 있다는 것이다.
					//		 따라서 인자가 있는 것과 인자가 없는 것에 호출 비율을 계산해 서버 관리자에게 보여준다면 서버의 송신 상태를 간접적으로 알려주게 될 것이다.

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
