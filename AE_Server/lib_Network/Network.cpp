#include "NetworkPCH.h"
#include "./Network.h"


CNetwork::CNetwork( __in INT iNetworkKind/* = NETWORK_KIND_DEFAULT*/ )
{
	m_dwNumberOfBytesRecvd = 0;
	m_bWasPendingRecv = false;
	m_bIsSD_RECEIVE = false;
	m_bIsSD_SEND = false;

	m_iNetworkKind = iNetworkKind;

	m_uni64NSocket = NULL;

	ZeroMemory( &m_stReceiveOverlappedEX, sizeof( m_stReceiveOverlappedEX ) );
	m_stReceiveOverlappedEX.IOType = IO_READ;
	m_stReceiveOverlappedEX.pParameter = this;

	ZeroMemory( &m_stSendOverlappedEX, sizeof( m_stSendOverlappedEX ) );
	m_stSendOverlappedEX.IOType = IO_WRITE;
	m_stSendOverlappedEX.pParameter = this;

	ZeroMemory( &m_stAcceptOverlappedEX, sizeof( m_stAcceptOverlappedEX ) );
	m_stAcceptOverlappedEX.IOType = IO_ACCEPT;
	m_stAcceptOverlappedEX.pParameter = this;
}

BOOL CNetwork::CreateNBOLSocket( VOID )
{
	if( m_uni64NSocket )
		return FALSE;

	m_uni64NSocket = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED );
	if( m_uni64NSocket == INVALID_SOCKET )
		return FALSE;

	return TRUE;
}

bool CNetwork::CreateConnection( __in LPSTR lpszAddr, __in USHORT unsPort )
{
	if( lpszAddr == NULL || unsPort == 0 )
		return false;

	if( m_uni64NSocket == NULL )
		return false;

	SOCKADDR_IN stRemoteAddrInfo;
	int iErrNo = 0;

	stRemoteAddrInfo.sin_family = AF_INET;
	stRemoteAddrInfo.sin_port = htons( unsPort );
	stRemoteAddrInfo.sin_addr.S_un.S_addr = inet_addr( lpszAddr );

	if( WSAConnect( m_uni64NSocket, ( LPSOCKADDR )&stRemoteAddrInfo, sizeof( SOCKADDR_IN ), NULL, NULL, NULL, NULL ) == SOCKET_ERROR )
	{
		iErrNo = WSAGetLastError();
		if( iErrNo != WSAEWOULDBLOCK )
		{
			TLOG( LOG_DEBUG, _T( "[ERROR] ErrNo.%d" ), iErrNo );
			return false;
		}
	}

	return true;
}

BOOL CNetwork::StartN( VOID )
{
	// 주의! 다중 Thread 동기화는 파생 Class에 Member 함수에서 한다.

	if( m_uni64NSocket )
		return FALSE;

	ZeroMemory( m_byReceiveBuffer, ( sizeof m_byReceiveBuffer ) );

	return TRUE;
}

BOOL CNetwork::EndN( VOID )
{
	// 주의! 다중 Thread 동기화는 파생 Class에 Member 함수에서 한다.

	if( ( m_uni64NSocket != 0 ) && ( closesocket( m_uni64NSocket ) == SOCKET_ERROR ) )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] ErrNo.%d. Failed to close the socket." ), WSAGetLastError() );
		return FALSE;
	}

	m_uni64NSocket = NULL;

	m_dwNumberOfBytesRecvd = 0;
	m_bWasPendingRecv = false;
	m_bIsSD_RECEIVE = false;
	m_bIsSD_SEND = false;

	return TRUE;
}

BOOL CNetwork::Listen( __in USHORT port, __in INT backLog )
{
	if (port <= 0 || backLog <= 0)
		return FALSE;

	if (!m_uni64NSocket)
		return FALSE;

	SOCKADDR_IN ListenSocketInfo;

	ListenSocketInfo.sin_family				= AF_INET;
	ListenSocketInfo.sin_port				= htons(port);
	ListenSocketInfo.sin_addr.S_un.S_addr	= htonl(INADDR_ANY);

	if (bind(m_uni64NSocket, (struct sockaddr*) &ListenSocketInfo, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		EndN();

		return FALSE;
	}

	if (listen(m_uni64NSocket, backLog) == SOCKET_ERROR)
	{
		EndN();

		return FALSE;
	}

	LINGER Linger;
	Linger.l_onoff	= 1;
	Linger.l_linger = 0;

	if (setsockopt(m_uni64NSocket, SOL_SOCKET, SO_LINGER, (char*) &Linger, sizeof(LINGER)) == SOCKET_ERROR)
	{
		EndN();

		return FALSE;
	}

	return TRUE;
}

BOOL CNetwork::Accept( __in SOCKET listenSocket )
{
	if( listenSocket == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] The listen socket(%#p) is invalid." ), listenSocket );
		return FALSE;
	}

	if( m_uni64NSocket != NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] The socket(%#p) already exists." ), m_uni64NSocket );
		return FALSE;
	}

	int iErrNo = 0;
	m_uni64NSocket = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED );
	if( m_uni64NSocket == INVALID_SOCKET )
	{
		iErrNo = WSAGetLastError();
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to create the socket. ErrNo.%d" ), iErrNo );
		return FALSE;
	}

	if( AcceptEx( listenSocket, m_uni64NSocket,
				  m_byReceiveBuffer, 0,
				  ( sizeof( sockaddr_in ) + 16 ), ( sizeof( sockaddr_in ) + 16 ),
				  NULL,
				  &m_stAcceptOverlappedEX.Overlapped ) == FALSE )
	{
		iErrNo = WSAGetLastError();
		if( iErrNo != ERROR_IO_PENDING )
		{
			TLOG( LOG_DEBUG, _T( "[ERROR] Failed to accept a new connection. ErrNo.%d" ), iErrNo );
			EndN();
			return FALSE;
		}
	}

	return TRUE;
}

bool CNetwork::HalfClose( __in INT iHow )
{
	if( SOCKET_ERROR == shutdown( GetNSocket(), iHow ) )
		TLOG( LOG_DEBUG, _T( "[WARNING] ErrNo.%d. Failed to disable sends or receives(%d) on a socket." ), WSAGetLastError(), iHow );
	else
	{
		switch( iHow )
		{
		case SD_RECEIVE:
			m_bIsSD_RECEIVE = true;
			break;

		case SD_SEND:
			m_bIsSD_SEND = true;
			break;

		case SD_BOTH:
			m_bIsSD_RECEIVE = true;
			m_bIsSD_SEND = true;
			break;
		}

		return true;
	}

	return false;
}

INT CNetwork::Receiving( bool bIsWSAES/* = false*/ )
{
	// 주의! 다중 Thread 동기화는 파생 Class에 Member 함수에서 한다.

	if( m_uni64NSocket == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Variable(%#p) is invalid." ), m_uni64NSocket );
		return NETWORK_IOC_STATE_FAILURE;
	}

	WSABUF stBuffers;
	INT iResult = 0;
	DWORD dwNumberOfBytesRecvd = 0, dwFlags = 0;

	stBuffers.buf = ( CHAR* )m_byReceiveBuffer;
	stBuffers.len = _countof( m_byReceiveBuffer );

	if( bIsWSAES == true )  // for 'WSAE'vent'S'elect
	{
		m_dwNumberOfBytesRecvd = 0;
		iResult = WSARecv( m_uni64NSocket, &stBuffers, 1, &m_dwNumberOfBytesRecvd, &dwFlags, &m_stReceiveOverlappedEX.Overlapped, NULL );
	}
	else  // for IOCP
		iResult = WSARecv( m_uni64NSocket, &stBuffers, 1, &dwNumberOfBytesRecvd, &dwFlags, &m_stReceiveOverlappedEX.Overlapped, NULL );
	if( iResult == SOCKET_ERROR )
	{
		iResult = WSAGetLastError();
		if( iResult == WSA_IO_PENDING )
			return NETWORK_IOC_STATE_PENDING;  // 수신 성공, 진행중
		else if( iResult == WSAEWOULDBLOCK )
			return NETWORK_IOC_STATE_RETRY;  // 수신 실패, 추후 재시도
		else
		{
			TLOG( LOG_DEBUG, _T( "[ERROR] Failed to receive packet. ErrNo.%d" ), iResult );
			return NETWORK_IOC_STATE_FAILURE;  // 수신 실패, 에러 발생
		}
	}

	return NETWORK_IOC_STATE_SUCCESS;  // 수신 성공, 완료
}

INT CNetwork::Sending( __in BYTE* pbyData, __in DWORD dwDataLength )
{
	// 주의! 다중 Thread 동기화는 파생 Class에 Member 함수에서 한다.

	if( pbyData == NULL || ( dwDataLength == 0 || PACKET_BUFFER_SIZE <= dwDataLength ) || m_uni64NSocket == NULL )
	{
		TLOG( LOG_DEBUG,
			  _T( "[ERROR] Parameter(%#p, %lu(0 < Length < %d), %#p) is invalid." ),
			  pbyData, dwDataLength, PACKET_BUFFER_SIZE, m_uni64NSocket );
		return NETWORK_IOC_STATE_FAILURE;
	}

	WSABUF stBuffers;
	DWORD dwNumberOfBytesSent = 0, dwFlags = 0;
	INT iResult = 0;

	stBuffers.buf = ( CHAR* )pbyData;
	stBuffers.len = dwDataLength;

	iResult = WSASend( m_uni64NSocket, &stBuffers, 1, &dwNumberOfBytesSent, dwFlags, &( m_stSendOverlappedEX.Overlapped ), NULL );
	if( iResult == SOCKET_ERROR )
	{
		iResult = WSAGetLastError();
		if( iResult == WSA_IO_PENDING )
			return NETWORK_IOC_STATE_PENDING;  // 송신 성공, 진행중
		else if( iResult == WSAEWOULDBLOCK )
			return NETWORK_IOC_STATE_RETRY;  // 송신 실패, 추후 재시도
		else
		{
			TLOG( LOG_DEBUG, _T( "[ERROR] Failed to send packet. ErrNo.%d" ), iResult );
			return NETWORK_IOC_STATE_FAILURE;  // 송신 실패, 에러 발생
		}
	}

	return NETWORK_IOC_STATE_SUCCESS;  // 송신 성공, 완료
}

VOID CNetwork::UpdateResultOfOverlappedRecv( VOID )
{
	// 주의! 다중 Thread 동기화는 파생 Class에 Member 함수에서 한다.

	BOOL bResult = FALSE;
	DWORD dwFlags = 0;
	INT iErrNo = 0;

	bResult = WSAGetOverlappedResult( m_uni64NSocket, &m_stReceiveOverlappedEX.Overlapped, &m_dwNumberOfBytesRecvd, m_bWasPendingRecv, &dwFlags );
	if( bResult == FALSE )
	{
		iErrNo = WSAGetLastError();
		TLOG( LOG_DEBUG, _T( "[ERROR] ErrNo.%d" ), iErrNo );
		m_dwNumberOfBytesRecvd = 0;
	}
}

INT CNetwork::QueuingReceivedData( __in BYTE* pbyEmptySection, __in INT nRemainingLength )
{
	// 주의! 다중 Thread 동기화는 파생 Class에 Member 함수에서 한다.

	size_t unn64TotalSizeOfRecvBuffer = ( sizeof m_byReceiveBuffer );

	if( 0 == m_dwNumberOfBytesRecvd || unn64TotalSizeOfRecvBuffer < m_dwNumberOfBytesRecvd )
	{
		TLOG( LOG_DEBUG,
			  _T( "[ERROR] Received size(%lu) is invalid for received data buffer(%u)." ),
			  m_dwNumberOfBytesRecvd, unn64TotalSizeOfRecvBuffer );
		// 주의! Packet이 누락될 수 있다.
		return -1;
	}

	if( nRemainingLength < ( ( INT )m_dwNumberOfBytesRecvd ) )
	{
		TLOG( LOG_DEBUG,
			  _T( "[ERROR] Received size(%lu) is bigger than a remaining space(%d)." ),
			  m_dwNumberOfBytesRecvd, nRemainingLength );
		return -2;
	}

	memcpy_s( pbyEmptySection, m_dwNumberOfBytesRecvd, m_byReceiveBuffer, m_dwNumberOfBytesRecvd );
	nRemainingLength -= m_dwNumberOfBytesRecvd;
	if( nRemainingLength < 0 )
	{
		TLOG( LOG_DEBUG,
			  _T( "[WARNING] It is strange!! Remaining length(%d) couldn't come out the negative number(Recvd length:%lu)." ),
			  nRemainingLength, m_dwNumberOfBytesRecvd );
	}
	ZeroMemory( m_byReceiveBuffer, unn64TotalSizeOfRecvBuffer );
	m_dwNumberOfBytesRecvd = 0;

	return nRemainingLength;
}
