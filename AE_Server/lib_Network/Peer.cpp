#include "NetworkPCH.h"
#include "./Network.h"
#include "./Peer.h"


BOOL CPeer::StartP( VOID )
{
	BOOL bResult = FALSE;

	SCOPED_LOCK( m_CSForReceiving )
	{
		m_nRemainingLength = ( sizeof m_byReceivedDataBuffer );
		ZeroMemory( m_byReceivedDataBuffer, m_nRemainingLength );

		bResult = CNetwork::StartN();
	}

	return bResult;
}

BOOL CPeer::EndP( VOID )
{
	BOOL bResult = FALSE;

	SCOPED_LOCK( m_CSForReceiving )
	{
		m_ReadPD.EndIOPD();
		m_WritePD.EndIOPD();

		bResult = CNetwork::EndN();
	}

	return bResult;
}

bool CPeer::ReadPacketForIOCP( __in DWORD dwReceiveSize/* = 0*/, __out BYTE* pbyReceivedPacket/* = NULL*/, __in ULONG_PTR ulProcessData/* = 0*/ )
{
	INT iResult = 0;

	// 주의! 수신 데이터에 대한 1차 적재 처리와 2차 적재 처리에 비율
	//		 수신 데이터를 CNetwork::m_byReceiveBuffer에서 CPeer::m_byReceivedDataBuffer로 옮기는 것을
	//		 1차 적재라고 한다면 CPeer::m_byReceivedDataBuffer에서 수신용 Queue로 옮기는 것을 2차 적재라고 하자.
	//		 1차 적재 처리와 2차 적재 처리에 비율은 Packet을 수신하는 과정에서 Hang이 발생하는 것을 줄이는데 도움이 될 것 같다.
	//		 여기서 고려된 비율은 '1차 적재 처리 : 2차 적재 처리 = 2 : 3'이다.
	//		 즉, 1차 적재 처리가 2번 이뤄지면 2차 적재 처리는 3번 이뤄지도록 구현하여 1차 적재에 대비하여
	//		 CPeer::m_byReceivedDataBuffer가 항상 비워진 상태가 될 수 있도록 유도하는 것이다.
	if( pbyReceivedPacket == NULL )  // 수신 후, 수신용 Queue에 삽입
	{
		if( dwReceiveSize )
		{
			CNetwork::m_dwNumberOfBytesRecvd = dwReceiveSize;

			if( IntegratedLoadForRawData() == false )
			{
				TLOG( LOG_DEBUG, _T( "[ERROR] Failed to receive packet." ) );
				return false;
			}
		}

		// 수신 요청
		if( m_bIsSD_RECEIVE == false )
		{
			iResult = CNetwork::Receiving();
			if( iResult == NETWORK_IOC_STATE_FAILURE )
			{
				TLOG( LOG_DEBUG, _T( "[ERROR] Failed to request a receiving." ) );
				return false;
			}
		}
	}
	else if( pbyReceivedPacket != NULL )  // Packet별 Local 처리를 위해 수신 Packet 추출
	{
		if( dwReceiveSize == 0 )
		{
			TLOG( LOG_DEBUG, _T( "[ERROR] Need to check. Parameter(%lu, %#p) is invalid." ), dwReceiveSize, pbyReceivedPacket );
			return false;
		}

		ULONG_PTR ulOffset = -1;
		LPVOID lpPacketOwner = NULL;
		BYTE* pbyExtractedUnitPacket = NULL;
		DWORD dwPacketLength = 0;

		SCOPED_LOCK( m_CS )
		{
			if( 0 == ulProcessData )
			{
				if( m_ReadPD.GetFromQueue( &ulOffset, &lpPacketOwner, &pbyExtractedUnitPacket, &dwPacketLength ) == false )
					return false;

				// 추출 값 검사
				if( ulOffset == -1 ||
					lpPacketOwner == NULL ||
					pbyExtractedUnitPacket == NULL ||
					( dwPacketLength == 0 || PACKET_BUFFER_SIZE < dwPacketLength ) )
				{
					TLOG( LOG_DEBUG, _T( "[ERROR] Need to check. Failed to extract into the read packet queue." ) );
					TLOG( LOG_DEBUG,
						_T( "          ulOffset : %#p, lpPacketOwner : %#p, pbyExtractedUnitPacket : %#p, dwPacketLength : %lu" ),
						  ulOffset, lpPacketOwner, pbyExtractedUnitPacket, dwPacketLength );
					dwPacketLength = 0;
					if( 0 < ulOffset )
						m_ReadPD.RemoveFromMap( ulOffset );
					m_ReadPD.PopFromQueue();
					return false;
				}

				memcpy_s( pbyReceivedPacket, dwReceiveSize, pbyExtractedUnitPacket, min( dwReceiveSize, dwPacketLength ) );
				m_ReadPD.RemoveFromMap( ulOffset );
				m_ReadPD.PopFromQueue();
			}
			else
			{
				ulOffset = ulProcessData;

				if( m_ReadPD.GetElementAtMemoryPool( ulOffset, &lpPacketOwner, &pbyExtractedUnitPacket, &dwPacketLength ) == false )
					return false;

				// 추출 값 검사
				if( ulOffset == -1 ||
					lpPacketOwner == NULL ||
					pbyExtractedUnitPacket == NULL ||
					( dwPacketLength == 0 || PACKET_BUFFER_SIZE < dwPacketLength ) )
				{
					TLOG( LOG_DEBUG, _T( "[ERROR] Need to check. Failed to extract into the read packet queue." ) );
					TLOG( LOG_DEBUG,
						_T( "          ulOffset : %#p, lpPacketOwner : %#p, pbyExtractedUnitPacket : %#p, dwPacketLength : %lu" ),
						  ulOffset, lpPacketOwner, pbyExtractedUnitPacket, dwPacketLength );
					dwPacketLength = 0;
					if( 0 < ulOffset )
						m_ReadPD.RemoveFromMap( ulOffset );
					return false;
				}

				memcpy_s( pbyReceivedPacket, dwReceiveSize, pbyExtractedUnitPacket, min( dwReceiveSize, dwPacketLength ) );
				m_ReadPD.RemoveFromMap( ulOffset );
			}  // if( 0 == ulProcessData ) else
		}  // SCOPED_LOCK( m_CS )
	}  // else if( pbyReceivedPacket != NULL )

	return true;
}

bool CPeer::ReadPacketForDOM( __in DWORD dwReceiveSize/* = 0*/ )
{
	INT iResult = 0;

	if( dwReceiveSize )
	{
		CNetwork::m_dwNumberOfBytesRecvd = dwReceiveSize;

		if( IntegratedLoadForRawData() == false )
		{
			TLOG( LOG_DEBUG, _T( "[ERROR] Failed to receive packet." ) );
			return false;
		}
	}

	// 수신 요청
	if( m_bIsSD_RECEIVE == false )
	{
		iResult = CNetwork::Receiving();
		if( iResult == NETWORK_IOC_STATE_FAILURE )
		{
			TLOG( LOG_DEBUG, _T( "[ERROR] Failed to request a receiving." ) );
			return false;
		}
	}

	return true;
}

bool CPeer::ReadPacketForWSAES( __out BYTE* pbyReceivedPacket/* = NULL*/ )
{
	INT iResult = 0;

	// 주의! 수신 데이터에 대한 1차 적재 처리와 2차 적재 처리에 비율
	//		 수신 데이터를 CNetwork::m_byReceiveBuffer에서 CPeer::m_byReceivedDataBuffer로 옮기는 것을
	//		 1차 적재라고 한다면 CPeer::m_byReceivedDataBuffer에서 수신용 Queue로 옮기는 것을 2차 적재라고 하자.
	//		 1차 적재 처리와 2차 적재 처리에 비율은 Packet을 수신하는 과정에서 Hang이 발생하는 것을 줄이는데 도움이 될 것 같다.
	//		 여기서 고려된 비율은 '1차 적재 처리 : 2차 적재 처리 = 2 : 3'이다.
	//		 즉, 1차 적재 처리가 2번 이뤄지면 2차 적재 처리는 3번 이뤄지도록 구현하여 1차 적재에 대비하여
	//		 CPeer::m_byReceivedDataBuffer가 항상 비워진 상태가 될 수 있도록 유도하는 것이다.
	if( pbyReceivedPacket )  // Packet별 Local 처리를 위해 수신 Packet 추출
	{
		ULONG_PTR ulOffset = -1;
		LPVOID lpPacketOwner = NULL;
		BYTE* pbyExtractedUnitPacket = NULL;
		DWORD dwPacketLength = 0;

		SCOPED_LOCK( m_CS )
		{
			if( m_ReadPD.GetFromQueue( &ulOffset, &lpPacketOwner, &pbyExtractedUnitPacket, &dwPacketLength ) == false )
				return false;

			// 추출 값 검사
			if( ulOffset == -1 ||
				lpPacketOwner == NULL ||
				pbyExtractedUnitPacket == NULL ||
				( dwPacketLength == 0 || PACKET_BUFFER_SIZE < dwPacketLength ) )
			{
				TLOG( LOG_DEBUG, _T( "[ERROR] Need to check. Failed to extract into the read packet queue." ) );
				TLOG( LOG_DEBUG,
					_T( "          ulOffset : %#p, lpPacketOwner : %#p, pbyExtractedUnitPacket : %#p, dwPacketLength : %lu" ),
					  ulOffset, lpPacketOwner, pbyExtractedUnitPacket, dwPacketLength );
				dwPacketLength = 0;
				if( 0 < ulOffset )
					m_ReadPD.RemoveFromMap( ulOffset );
				m_ReadPD.PopFromQueue();
				return false;
			}

			// 주의! pbyReceivedPacket에 크기를 알 수 없으므로 Buffer overflow가 발생할 가능성이 있다.
			// TODO: pbyReceivedPacket에 크기를 알 수 있도록 수정해야 한다.
			CopyMemory( pbyReceivedPacket, pbyExtractedUnitPacket, dwPacketLength );
			m_ReadPD.RemoveFromMap( ulOffset );
			m_ReadPD.PopFromQueue();
		}
	}
	else  // 수신 후, 수신용 Queue에 삽입
	{
		// 주의! FD_READ에 따라 CNetwork::Receiving()이 호출되기 전인
		//		 if( CNetwork::m_bWasPendingRecv ) Block 안에서 return하는 것은 신중해야 한다.
		// WSA_IO_PENDING에 대한 처리
		if( CNetwork::m_bWasPendingRecv )
		{
			CNetwork::UpdateResultOfOverlappedRecv();

			if( IntegratedLoadForRawData() == false )
			{
				TLOG( LOG_DEBUG, _T( "[ERROR] Failed to receive packet." ) );
				return false;
			}

			CNetwork::m_bWasPendingRecv = false;
		}

		// 수신
		iResult = CNetwork::Receiving( true );
		if( iResult == NETWORK_IOC_STATE_SUCCESS )
		{
			if( IntegratedLoadForRawData() == false )
				return false;

			CNetwork::m_bWasPendingRecv = false;
		}
		else if( iResult == NETWORK_IOC_STATE_PENDING )
			CNetwork::m_bWasPendingRecv = true;
		else
		{
			TLOG( LOG_DEBUG,
				  _T( "[ERROR] Return value is %s." ),
				  ( ( iResult == NETWORK_IOC_STATE_RETRY )? _T( "WSAEWOULDBLOCK" ):_T( "Failure" ) ) );
			// TODO: NETWORK_IOC_STATE_FAILURE인 경우,
			//		 CNetwork에서 CNetwork::EndN()을 호출하여 접속을 끊으므로
			//		 이와 관련된 데이터(큐,맵,기타등등)는 모두 제거되어야 하는데...
			//		 NETWORK_IOC_STATE_RETRY인 경우엔 기존 처리 그대로 한다.
			// TODO: closesocket()가 호출되었을 때, WSAEventSelect에서는 어떤 이벤트가
			//		 발생하는지 확인해야 한다. FD_CLOSE가 발생한다면 FD_CLOSE 처리에서
			//		 끊어진 소캣에 대한 객체 정리 작업을 해야 하고 FD_CLOSE가 발생하지
			//		 않으면 다른 적절한 곳에서 끊어진 소캣에 대한 객체 정리 작업을 해야 한다.
			CNetwork::m_bWasPendingRecv = false;
			return false;
		}
	}

	return true;
}

INT CPeer::WritePacket( __in WORD wSendSize/* = 0*/, __in BYTE* pbySendData/* = NULL*/ )
{
	ULONG_PTR ulOffset = -1;
	BYTE* pbyToSendData = NULL;
	INT iResult = NETWORK_IOC_STATE_DEFAULT;

	m_WritePD.RemoveFromMap( NETWORK_IOC_STATE_FAILURE );

	// TODO: 패킷 송신 순서를 보장하지 못하는 문제
	//		 '즉시 송신'은 작업 쓰레드(TSession::PacketThreadCallback())에서 호출되고
	//		 '순환 송신'은 WSAEventSelect 모델 쓰레드(CWSAEventSelect::WSAESThreadCallback())에서 호출되므로
	//		 송신 큐에 먼저 송신해야 할 패킷들이 쌓여 있음에도 불구하고 절묘한(?) 타이밍으로 '즉시 송신'이 먼저 이뤄져
	//		 패킷 송신 순서가 뒤바뀔 수 있는 문제가 우려된다. 여기서 말하는 절묘한 타이밍이 발생할 수 있는 예상되는 상황은
	//		 FD_WRITE가 발생하는 시점에 '즉시 송신'을 하는 함수가 '순환 송신'을 하는 함수보다 먼저 호출되는 것이다.
	//		 즉, '즉시 송신'을 하는 쓰레드는 FD_WRITE가 발생하는 것에 대해서 구조적으로 전혀 알 수 없다. 하지만,
	//		 '순환 송신'을 하는 쓰레드는 구조적으로 FD_WRITE을 감지하게 되어 있다. 이런 상황에서 커널(?)에서 FD_WRITE를 발신하고
	//		 '순환 송신' 쓰레드가 이를 받기까지 아주 짧은 순간에 '즉신 송시' 쓰레드에서 '즉시 송신' 함수를 호출하였다.
	//		 그리고 FD_WRITE 신호는 '즉시 송신' 함수가 호출된 이후에 '순환 송신' 쓰레드가 받았다.
	//		 '즉시 송신' 처리되는 패킷 보다 '순환 송신'에 의해 송신용 큐에 쌓인 패킷이 먼저 송신되어야 하는 것이 정상이나
	//		 이런 상황에서는 이 순서가 뒤바뀌게 된다. 이에 대한 해결책으로는 '즉시 송신'에서는 WSASend()를 호출하지 않고
	//		 송신 큐에 송신 하려는 패킷을 삽입하는 처리만 하도록 하며, WSASend() 호출은 '순환 송신'에서만 호출하도록 하는 것으로
	//		 문제를 해결할 수 있을 것 같다.

	if( ( wSendSize != 0 ) && ( pbySendData != NULL ) )  // 즉시 송신
	{
		if( m_bIsSD_SEND == true )
			return iResult;

		bool bResult = false;

		// 송신 Packet 검사
		if( PACKET_BUFFER_SIZE <= wSendSize )
		{
			TLOG( LOG_DEBUG,
				_T( "[ERROR] Parameter(%d(0 < Length < %d), %#p) is invalid." ),
				  wSendSize, PACKET_BUFFER_SIZE, pbySendData );
			TLOG( LOG_DEBUG, _T( "Failed to send packet." ) );
			return NETWORK_IOC_STATE_FAILURE;
		}

		// 처리해야 하는 Packet 보관
		bResult = m_WritePD.BorrowFromMemoryPool( this, pbySendData, wSendSize, &ulOffset, &pbyToSendData );
		if( bResult == false || ulOffset == -1 || pbyToSendData == NULL )
		{
			TLOG( LOG_DEBUG, _T( "[ERROR] Failed to borrow the store space in memory pool." ) );
			TLOG( LOG_DEBUG, _T( "[ERROR] Failed to send packet." ) );
			return NETWORK_IOC_STATE_FAILURE;
		}
		m_WritePD.InsertIntoMap( ulOffset, NETWORK_IOC_STATE_DEFAULT );

		// 송신
		SCOPED_LOCK( m_CSForSending )
		{
			iResult = CNetwork::Sending( pbyToSendData, wSendSize );
		}
		m_WritePD.UpdateMap( ulOffset, iResult );
		switch( iResult )
		{
		case NETWORK_IOC_STATE_SUCCESS:
			m_WritePD.RemoveFromMap( NETWORK_IOC_STATE_SUCCESS );
			break;

		case NETWORK_IOC_STATE_FAILURE:
			TLOG( LOG_DEBUG, _T( "[ERROR] The failed packet protocol is %d." ), *( ( WORD* )( pbyToSendData + 2 ) ) );  // UDP_HEADER 구조가 아닌 경우 Protocol
			// TODO: NETWORK_IOC_STATE_FAILURE인 경우,
			//		 CNetwork에서 CNetwork::EndN()을 호출하여 접속을 끊으므로
			//		 이와 관련된 데이터(큐,맵,기타등등)는 모두 제거되어야 하는데...
			// TODO: closesocket()가 호출되었을 때, WSAEventSelect에서는 어떤 이벤트가
			//		 발생하는지 확인해야 한다. FD_CLOSE가 발생한다면 FD_CLOSE 처리에서
			//		 끊어진 소캣에 대한 객체 정리 작업을 해야 하고 FD_CLOSE가 발생하지
			//		 않으면 다른 적절한 곳에서 끊어진 소캣에 대한 객체 정리 작업을 해야 한다.
			break;

		case NETWORK_IOC_STATE_RETRY:  // WSAEWOULDBLOCK에 대한 처리, 송신용 Queue 삽입
			m_WritePD.PushIntoQueue( ulOffset );
			break;
		}
	}  // if( ( wSendSize != 0 ) && ( pbySendData != NULL ) )
	else  // 미송신되어 Queue에 쌓인 Packet, 순환 송신
	{
		if( ( wSendSize != 0 ) || ( pbySendData != NULL ) )
		{
			TLOG( LOG_DEBUG,
				_T( "[WARNING] Need to check the parameter(%d(0 < Length < %d), %#p)." ),
				  wSendSize, PACKET_BUFFER_SIZE, pbySendData );
		}

		LPVOID lpPacketOwner = NULL;
		DWORD dwPacketLength = 0;

		m_WritePD.RemoveFromMap( NETWORK_IOC_STATE_SUCCESS );
		m_WritePD.RemoveFromMap( NETWORK_IOC_STATE_PENDING );

		if( m_bIsSD_SEND == true )
			return iResult;

		SCOPED_LOCK( m_CSForSending )
		{
			while( m_WritePD.GetFromQueue( &ulOffset, &lpPacketOwner, &pbyToSendData, &dwPacketLength ) )
			{
				// 추출 값 검사
				if( ulOffset == -1 ||
					lpPacketOwner == NULL ||
					pbyToSendData == NULL ||
					( dwPacketLength == 0 || PACKET_BUFFER_SIZE < dwPacketLength ) )
				{
					TLOG( LOG_DEBUG, _T( "[ERROR] Need to check. Failed to insert into the write packet queue." ) );
					dwPacketLength = 0;
					m_WritePD.UpdateMap( ulOffset, NETWORK_IOC_STATE_FAILURE );
					m_WritePD.PopFromQueue();
					continue;
				}

				// 송신
				iResult = CNetwork::Sending( pbyToSendData, dwPacketLength );
				m_WritePD.UpdateMap( ulOffset, iResult );
				switch( iResult )
				{
				case NETWORK_IOC_STATE_SUCCESS:
					m_WritePD.RemoveFromMap( NETWORK_IOC_STATE_SUCCESS );
					break;

				case NETWORK_IOC_STATE_FAILURE:
					TLOG( LOG_DEBUG, _T( "[ERROR] The failed packet protocol is %d." ), *( ( WORD* )( pbyToSendData + 2 ) ) );  // UDP_HEADER 구조가 아닌 경우 Protocol
					// TODO: NETWORK_IOC_STATE_FAILURE인 경우,
					//		 CNetwork에서 CNetwork::EndN()을 호출하여 접속을 끊으므로
					//		 이와 관련된 데이터(큐,맵,기타등등)는 모두 제거되어야 하는데...
					//		 아래 m_WritePD.PopFromQueue()에 대해서 다시 한번 생각해보자.
					// TODO: closesocket()가 호출되었을 때, WSAEventSelect에서는 어떤 이벤트가
					//		 발생하는지 확인해야 한다. FD_CLOSE가 발생한다면 FD_CLOSE 처리에서
					//		 끊어진 소캣에 대한 객체 정리 작업을 해야 하고 FD_CLOSE가 발생하지
					//		 않으면 다른 적절한 곳에서 끊어진 소캣에 대한 객체 정리 작업을 해야 한다.
					return NETWORK_IOC_STATE_FAILURE;

				case NETWORK_IOC_STATE_RETRY:  // WSAEWOULDBLOCK에 대한 처리, 다음에 재시도
					return NETWORK_IOC_STATE_RETRY;
				}

				m_WritePD.PopFromQueue();

				// 초기화
				ulOffset = -1;
				lpPacketOwner = NULL;
				pbyToSendData = NULL;
				dwPacketLength = 0;
			}  // while( m_WritePD.GetFromQueue( &ulOffset, &lpPacketOwner, &pbyToSendData, &dwPacketLength ) )
		}  // SCOPED_LOCK( m_CSForSending )
	}  // if( ( wSendSize != 0 ) && ( pbySendData != NULL ) ) else

	return iResult;
}

bool CPeer::InsertREQDataIntoMainQ( __in WORD wREQDataSize, __in BYTE* pbyREQDataStartADR )
{
	BYTE pbyUnitPacket[PACKET_BUFFER_SIZE] = {0,};
	bool bResult = false;
	ULONG_PTR ulOffset = -1;
	BYTE* pbyStoredData = NULL;

	SCOPED_LOCK( m_CSForReceiving )
	{
		memcpy_s( pbyUnitPacket, PACKET_BUFFER_SIZE, pbyREQDataStartADR, wREQDataSize );

		if( false == ChangePacketProcessingMode( pbyUnitPacket ) )
		{
			// Memory pool에서 공간 빌림
			bResult = m_ReadPD.BorrowFromMemoryPool( this, pbyUnitPacket, wREQDataSize, &ulOffset, &pbyStoredData );
			if( bResult == false || ulOffset == -1 || pbyStoredData == NULL )
			{
				TLOG( LOG_DEBUG, _T( "[ERROR] Failed to borrow the store space in memory pool." ) );
				bResult = false;
			}
			else
			{
				m_ReadPD.InsertIntoMap( ulOffset, NETWORK_IOC_STATE_DEFAULT );

				// Queue에 삽입
				if( ThrowToMainQ( ulOffset ) )
					m_ReadPD.PushIntoQueue( ulOffset );  // TODO: 메인 큐를 사용하게 되면 이 처리가 필요 없다.
				// TODO: ThrowToMainQ()의 반환값은 VOID로 한다. 하지만, 여기서는 bool로 하고 있다.
				//		 이는 편법이며 전 서버에 메인 스레드 & 메인 큐가 적용되면 ThrowToMain()의 반환값은 VOID로 하고
				//		 ThrowToMain()이 호출되므로 m_ReadPD.PushIntoQueue()은 호출할 필요 없다.
				//		 ThrowToMain()을 호출하면서 m_ReadPD.PushIntoQueue()을 호출하게 되면
				//		 CIOPacketDepository::m_queueWaitingToProcess에 불필요한 메모리 확보가 발생하고 해제되지 않는다.
				//		 ThrowToMainQ()의 반환값을 bool로 한 것은 ThrowToMainQ()이 재정의된 서버에서는 m_ReadPD.PushIntoQueue()을 호출하지 않기 위해서다.
				//		 ThrowToMainQ()이 재정의된 서버는 메인 스레드 & 메인 큐가 적용된 서버다.
			}
		}
		else
			bResult = true;
	}

	return bResult;
}

bool CPeer::IntegratedLoadForRawData( VOID )
{
	BYTE* pbyEmptySectionOfRecvdBuffer = NULL;
	USHORT unsPrevPacketLength = 0, unsPacketLength = 0;
	INT nPrevRemainingLength = -1, nRepeatingCounter = 0, nRemainingLength = -1, n2ndLoadUpResult = 1;

	SCOPED_LOCK( m_CSForReceiving )
	{
		nPrevRemainingLength = m_nRemainingLength;
		unsPrevPacketLength = MAKEWORD( m_byReceivedDataBuffer[0], m_byReceivedDataBuffer[1] );

		do
		{
			++nRepeatingCounter;

			// 1차 적재
			pbyEmptySectionOfRecvdBuffer = m_byReceivedDataBuffer + ( ( sizeof m_byReceivedDataBuffer ) - m_nRemainingLength );
			nRemainingLength = CNetwork::QueuingReceivedData( pbyEmptySectionOfRecvdBuffer, m_nRemainingLength );
			if( 0 <= nRemainingLength )
				m_nRemainingLength = nRemainingLength;
			else
			{
				unsPacketLength = MAKEWORD( m_byReceivedDataBuffer[0], m_byReceivedDataBuffer[1] );

				TLOG( LOG_DEBUG, _T( "[WARNING] Can not put received data on the received data buffer(%d)." ), nRemainingLength );
				TLOG( LOG_DEBUG,
					_T( "          m_byReceivedDataBuffer : %#p, nPrevRemainingLength : %d, m_nRemainingLength : %d" ),
					  m_byReceivedDataBuffer, nPrevRemainingLength, m_nRemainingLength );
				TLOG( LOG_DEBUG,
					  _T( "          unsPrevPacketLength : %u, unsPacketLength : %u, nRepeatingCounter : %d" ),
					  unsPrevPacketLength, unsPacketLength, nRepeatingCounter );
			}

			// 2차 적재
			n2ndLoadUpResult = LoadUpIntoQueue();
			if( n2ndLoadUpResult == 0 )  // 반환 값 -1과 1은 성공
				return false;
			else if( n2ndLoadUpResult == -1 && nRemainingLength == -2 )  // 주의! 이 경우 무한 Loop
			{
				unsPacketLength = MAKEWORD( m_byReceivedDataBuffer[0], m_byReceivedDataBuffer[1] );

				TLOG( LOG_DEBUG, _T( "[ERROR] Failed to load up the packet unit into queue. Disconnecting with the opposite side(%#p)." ), this );
				TLOG( LOG_DEBUG,
					  _T( "          m_byReceivedDataBuffer : %#p, nPrevRemainingLength : %d, m_nRemainingLength : %d" ),
					  m_byReceivedDataBuffer, nPrevRemainingLength, m_nRemainingLength );
				TLOG( LOG_DEBUG,
					  _T( "          unsPrevPacketLength : %u, unsPacketLength : %u, nRepeatingCounter : %d" ),
					  unsPrevPacketLength, unsPacketLength, nRepeatingCounter );
				// NOTE: 호출한 측에서 상대방과 연결 종료 처리
				return false;
			}
		}
		while( nRemainingLength == -2 );
		// 주의! 무한 Loop에 빠지지 않도록 주의해야 한다.
	}

	return true;
}

INT CPeer::LoadUpIntoQueue( VOID )
{
	while( true )
	{
		// 2차 적재 준비
		size_t unnTotalSizeOfRecvdBuffer = ( sizeof m_byReceivedDataBuffer );
		size_t unnUsingSize = ( unnTotalSizeOfRecvdBuffer - m_nRemainingLength );
		if( unnUsingSize < 2 )
			break;

		USHORT unsPacketLength = MAKEWORD( m_byReceivedDataBuffer[0], m_byReceivedDataBuffer[1] );
		if( unsPacketLength == 0 )  // 연결 직후, 아직 수신된 Data가 없는 경우
		{
			if( 0 < unnUsingSize )
			{
				TLOG( LOG_DEBUG,
					  _T( "[ERROR] There is no packet to be processed. But, unprocessed data remains in the received data buffer. Using size : %u" ),
					  unnUsingSize );
			}
			return -1;
		}

		if( PACKET_BUFFER_SIZE < unsPacketLength )
		{
			TLOG( LOG_DEBUG,
				  _T( "[ERROR] The size(0 < Length(%u) <= %d) of packet is invalid." ),
				  unsPacketLength, PACKET_BUFFER_SIZE );
			// NOTE: 호출한 측에서 상대방과 연결 종료 처리

			// TODO: 만약 pbyUnitPacket에 담을 수 없는 크기의 패킷을 받은 것이라면?
			//		 즉, 크기만 클뿐이지 정상적인 패킷이라면 어떻게 해야할까? 여기서는 이를 담아낼 공간이 없으니 처리할 수 없다.
			//		 그러면 연결을 끊지 말고 문제가 되는 크기만큼 수신된 패킷 데이터를 제거하고 다음 패킷부터 처리한다?
			return 0;
		}

		// 2차 적재 시작
		if( unsPacketLength <= unnUsingSize )
		{
			BYTE pbyUnitPacket[PACKET_BUFFER_SIZE] = {0,};
			bool bResult = false;
			ULONG_PTR ulOffset = -1;
			BYTE* pbyStoredData = NULL;
			SIZE_T unlSizeNeededToProcess = unnUsingSize - unsPacketLength;

			memcpy_s( pbyUnitPacket, PACKET_BUFFER_SIZE, m_byReceivedDataBuffer, unsPacketLength );

			// NOTE: ChangePacketProcessingMode()에 의해 소켓 관련 데이터가 갑자기 전혀 다른 값을 가질 수 있다!
			//		 이곳은 패킷에 대한 처리를 하기 전 패킷 처리의 재료가 되는 수신된 데이터를 단일 패킷으로 구성하거나
			//		 관련 버퍼를 관리하는 곳이다. 다시 말해 수신된 Raw Data를 처리 중인 곳이다.
			//
			//		 Main Q와 Main Thread를 사용하지 않는 영지 모드 서버가 생기면서 원래는 없었던 ChangePacket-
			//		 ProcessingMode()을 통해 수신된 Raw Data를 처리 중인 이곳에서 패킷 처리가 이뤄지게 됐다.
			//		 이로 인해 연결 종료와 관련된 Raw Data를 처리 중에 ChangePacketProcessingMode()에서 연결 종료와
			//		 관련된 패킷을 처리하게 되면 Raw Data 처리가 아직 끝나지도 않았는데 소켓을 초기화 해버리는 문제가
			//		 발생할 수 있다. 이런 상황은 '종료 처리 로직'에 의해 희박하지만 발생할 수 있으므로 주의해야 한다.
			//		 ※ Redmine #192 '종료 처리' 참고
			if( false == ChangePacketProcessingMode( pbyUnitPacket ) )
			{
				// Memory pool에서 공간 빌림
				bResult = m_ReadPD.BorrowFromMemoryPool( this, pbyUnitPacket, unsPacketLength, &ulOffset, &pbyStoredData );
				if( bResult == false || ulOffset == -1 || pbyStoredData == NULL )
				{
					TLOG( LOG_DEBUG, _T( "[ERROR] Failed to borrow the store space in memory pool." ) );
					// NOTE: 호출한 측에서 상대방과 연결 종료 처리
					return 0;
				}
				else
				{
					m_ReadPD.InsertIntoMap( ulOffset, NETWORK_IOC_STATE_DEFAULT );

					// Queue에 삽입
					if( ThrowToMainQ( ulOffset ) )
						m_ReadPD.PushIntoQueue( ulOffset );  // TODO: 메인 큐를 사용하게 되면 이 처리가 필요 없다.
					// TODO: ThrowToMainQ()의 반환값은 VOID로 한다. 하지만, 여기서는 bool로 하고 있다.
					//		 이는 편법이며 전 서버에 메인 스레드 & 메인 큐가 적용되면 ThrowToMain()의 반환값은 VOID로 하고
					//		 ThrowToMain()이 호출되므로 m_ReadPD.PushIntoQueue()은 호출할 필요 없다.
					//		 ThrowToMain()을 호출하면서 m_ReadPD.PushIntoQueue()을 호출하게 되면
					//		 CIOPacketDepository::m_queueWaitingToProcess에 불필요한 메모리 확보가 발생하고 해제되지 않는다.
					//		 ThrowToMainQ()의 반환값을 bool로 한 것은 ThrowToMainQ()이 재정의된 서버에서는 m_ReadPD.PushIntoQueue()을 호출하지 않기 위해서다.
					//		 ThrowToMainQ()이 재정의된 서버는 메인 스레드 & 메인 큐가 적용된 서버다.
				}
			}

			// 다음에 1차 적재를 위해 Buffer 정리
			MoveMemory( m_byReceivedDataBuffer, ( m_byReceivedDataBuffer + unsPacketLength ), unlSizeNeededToProcess );

			m_nRemainingLength += unsPacketLength;
			if( ( unnTotalSizeOfRecvdBuffer < static_cast<size_t>( m_nRemainingLength ) ) ||
				( unnTotalSizeOfRecvdBuffer < ( unlSizeNeededToProcess + static_cast<size_t>( m_nRemainingLength ) ) ) )
			{
				TLOG( LOG_DEBUG,
					  _T( "[ERROR] It will be disconnected with the player because of memory access violation." ) );
				TLOG( LOG_DEBUG,
					  _T( "          Received data buffer's Max size : %Iu | Using size : %Iu | Remaining size : %Id" ),
					  unnTotalSizeOfRecvdBuffer, unlSizeNeededToProcess, m_nRemainingLength );
				return 0;
			}
			ZeroMemory( m_byReceivedDataBuffer + unlSizeNeededToProcess, m_nRemainingLength );
		}
		else
			break;
	}  // while( true )

	return 1;
}
