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

	// ����! ���� �����Ϳ� ���� 1�� ���� ó���� 2�� ���� ó���� ����
	//		 ���� �����͸� CNetwork::m_byReceiveBuffer���� CPeer::m_byReceivedDataBuffer�� �ű�� ����
	//		 1�� ������ �Ѵٸ� CPeer::m_byReceivedDataBuffer���� ���ſ� Queue�� �ű�� ���� 2�� ������ ����.
	//		 1�� ���� ó���� 2�� ���� ó���� ������ Packet�� �����ϴ� �������� Hang�� �߻��ϴ� ���� ���̴µ� ������ �� �� ����.
	//		 ���⼭ ����� ������ '1�� ���� ó�� : 2�� ���� ó�� = 2 : 3'�̴�.
	//		 ��, 1�� ���� ó���� 2�� �̷����� 2�� ���� ó���� 3�� �̷������� �����Ͽ� 1�� ���翡 ����Ͽ�
	//		 CPeer::m_byReceivedDataBuffer�� �׻� ����� ���°� �� �� �ֵ��� �����ϴ� ���̴�.
	if( pbyReceivedPacket == NULL )  // ���� ��, ���ſ� Queue�� ����
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

		// ���� ��û
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
	else if( pbyReceivedPacket != NULL )  // Packet�� Local ó���� ���� ���� Packet ����
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

				// ���� �� �˻�
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

				// ���� �� �˻�
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

	// ���� ��û
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

	// ����! ���� �����Ϳ� ���� 1�� ���� ó���� 2�� ���� ó���� ����
	//		 ���� �����͸� CNetwork::m_byReceiveBuffer���� CPeer::m_byReceivedDataBuffer�� �ű�� ����
	//		 1�� ������ �Ѵٸ� CPeer::m_byReceivedDataBuffer���� ���ſ� Queue�� �ű�� ���� 2�� ������ ����.
	//		 1�� ���� ó���� 2�� ���� ó���� ������ Packet�� �����ϴ� �������� Hang�� �߻��ϴ� ���� ���̴µ� ������ �� �� ����.
	//		 ���⼭ ����� ������ '1�� ���� ó�� : 2�� ���� ó�� = 2 : 3'�̴�.
	//		 ��, 1�� ���� ó���� 2�� �̷����� 2�� ���� ó���� 3�� �̷������� �����Ͽ� 1�� ���翡 ����Ͽ�
	//		 CPeer::m_byReceivedDataBuffer�� �׻� ����� ���°� �� �� �ֵ��� �����ϴ� ���̴�.
	if( pbyReceivedPacket )  // Packet�� Local ó���� ���� ���� Packet ����
	{
		ULONG_PTR ulOffset = -1;
		LPVOID lpPacketOwner = NULL;
		BYTE* pbyExtractedUnitPacket = NULL;
		DWORD dwPacketLength = 0;

		SCOPED_LOCK( m_CS )
		{
			if( m_ReadPD.GetFromQueue( &ulOffset, &lpPacketOwner, &pbyExtractedUnitPacket, &dwPacketLength ) == false )
				return false;

			// ���� �� �˻�
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

			// ����! pbyReceivedPacket�� ũ�⸦ �� �� �����Ƿ� Buffer overflow�� �߻��� ���ɼ��� �ִ�.
			// TODO: pbyReceivedPacket�� ũ�⸦ �� �� �ֵ��� �����ؾ� �Ѵ�.
			CopyMemory( pbyReceivedPacket, pbyExtractedUnitPacket, dwPacketLength );
			m_ReadPD.RemoveFromMap( ulOffset );
			m_ReadPD.PopFromQueue();
		}
	}
	else  // ���� ��, ���ſ� Queue�� ����
	{
		// ����! FD_READ�� ���� CNetwork::Receiving()�� ȣ��Ǳ� ����
		//		 if( CNetwork::m_bWasPendingRecv ) Block �ȿ��� return�ϴ� ���� �����ؾ� �Ѵ�.
		// WSA_IO_PENDING�� ���� ó��
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

		// ����
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
			// TODO: NETWORK_IOC_STATE_FAILURE�� ���,
			//		 CNetwork���� CNetwork::EndN()�� ȣ���Ͽ� ������ �����Ƿ�
			//		 �̿� ���õ� ������(ť,��,��Ÿ���)�� ��� ���ŵǾ�� �ϴµ�...
			//		 NETWORK_IOC_STATE_RETRY�� ��쿣 ���� ó�� �״�� �Ѵ�.
			// TODO: closesocket()�� ȣ��Ǿ��� ��, WSAEventSelect������ � �̺�Ʈ��
			//		 �߻��ϴ��� Ȯ���ؾ� �Ѵ�. FD_CLOSE�� �߻��Ѵٸ� FD_CLOSE ó������
			//		 ������ ��Ĺ�� ���� ��ü ���� �۾��� �ؾ� �ϰ� FD_CLOSE�� �߻�����
			//		 ������ �ٸ� ������ ������ ������ ��Ĺ�� ���� ��ü ���� �۾��� �ؾ� �Ѵ�.
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

	// TODO: ��Ŷ �۽� ������ �������� ���ϴ� ����
	//		 '��� �۽�'�� �۾� ������(TSession::PacketThreadCallback())���� ȣ��ǰ�
	//		 '��ȯ �۽�'�� WSAEventSelect �� ������(CWSAEventSelect::WSAESThreadCallback())���� ȣ��ǹǷ�
	//		 �۽� ť�� ���� �۽��ؾ� �� ��Ŷ���� �׿� �������� �ұ��ϰ� ������(?) Ÿ�̹����� '��� �۽�'�� ���� �̷���
	//		 ��Ŷ �۽� ������ �ڹٲ� �� �ִ� ������ ����ȴ�. ���⼭ ���ϴ� ������ Ÿ�̹��� �߻��� �� �ִ� ����Ǵ� ��Ȳ��
	//		 FD_WRITE�� �߻��ϴ� ������ '��� �۽�'�� �ϴ� �Լ��� '��ȯ �۽�'�� �ϴ� �Լ����� ���� ȣ��Ǵ� ���̴�.
	//		 ��, '��� �۽�'�� �ϴ� ������� FD_WRITE�� �߻��ϴ� �Ϳ� ���ؼ� ���������� ���� �� �� ����. ������,
	//		 '��ȯ �۽�'�� �ϴ� ������� ���������� FD_WRITE�� �����ϰ� �Ǿ� �ִ�. �̷� ��Ȳ���� Ŀ��(?)���� FD_WRITE�� �߽��ϰ�
	//		 '��ȯ �۽�' �����尡 �̸� �ޱ���� ���� ª�� ������ '��� �۽�' �����忡�� '��� �۽�' �Լ��� ȣ���Ͽ���.
	//		 �׸��� FD_WRITE ��ȣ�� '��� �۽�' �Լ��� ȣ��� ���Ŀ� '��ȯ �۽�' �����尡 �޾Ҵ�.
	//		 '��� �۽�' ó���Ǵ� ��Ŷ ���� '��ȯ �۽�'�� ���� �۽ſ� ť�� ���� ��Ŷ�� ���� �۽ŵǾ�� �ϴ� ���� �����̳�
	//		 �̷� ��Ȳ������ �� ������ �ڹٲ�� �ȴ�. �̿� ���� �ذ�å���δ� '��� �۽�'������ WSASend()�� ȣ������ �ʰ�
	//		 �۽� ť�� �۽� �Ϸ��� ��Ŷ�� �����ϴ� ó���� �ϵ��� �ϸ�, WSASend() ȣ���� '��ȯ �۽�'������ ȣ���ϵ��� �ϴ� ������
	//		 ������ �ذ��� �� ���� �� ����.

	if( ( wSendSize != 0 ) && ( pbySendData != NULL ) )  // ��� �۽�
	{
		if( m_bIsSD_SEND == true )
			return iResult;

		bool bResult = false;

		// �۽� Packet �˻�
		if( PACKET_BUFFER_SIZE <= wSendSize )
		{
			TLOG( LOG_DEBUG,
				_T( "[ERROR] Parameter(%d(0 < Length < %d), %#p) is invalid." ),
				  wSendSize, PACKET_BUFFER_SIZE, pbySendData );
			TLOG( LOG_DEBUG, _T( "Failed to send packet." ) );
			return NETWORK_IOC_STATE_FAILURE;
		}

		// ó���ؾ� �ϴ� Packet ����
		bResult = m_WritePD.BorrowFromMemoryPool( this, pbySendData, wSendSize, &ulOffset, &pbyToSendData );
		if( bResult == false || ulOffset == -1 || pbyToSendData == NULL )
		{
			TLOG( LOG_DEBUG, _T( "[ERROR] Failed to borrow the store space in memory pool." ) );
			TLOG( LOG_DEBUG, _T( "[ERROR] Failed to send packet." ) );
			return NETWORK_IOC_STATE_FAILURE;
		}
		m_WritePD.InsertIntoMap( ulOffset, NETWORK_IOC_STATE_DEFAULT );

		// �۽�
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
			TLOG( LOG_DEBUG, _T( "[ERROR] The failed packet protocol is %d." ), *( ( WORD* )( pbyToSendData + 2 ) ) );  // UDP_HEADER ������ �ƴ� ��� Protocol
			// TODO: NETWORK_IOC_STATE_FAILURE�� ���,
			//		 CNetwork���� CNetwork::EndN()�� ȣ���Ͽ� ������ �����Ƿ�
			//		 �̿� ���õ� ������(ť,��,��Ÿ���)�� ��� ���ŵǾ�� �ϴµ�...
			// TODO: closesocket()�� ȣ��Ǿ��� ��, WSAEventSelect������ � �̺�Ʈ��
			//		 �߻��ϴ��� Ȯ���ؾ� �Ѵ�. FD_CLOSE�� �߻��Ѵٸ� FD_CLOSE ó������
			//		 ������ ��Ĺ�� ���� ��ü ���� �۾��� �ؾ� �ϰ� FD_CLOSE�� �߻�����
			//		 ������ �ٸ� ������ ������ ������ ��Ĺ�� ���� ��ü ���� �۾��� �ؾ� �Ѵ�.
			break;

		case NETWORK_IOC_STATE_RETRY:  // WSAEWOULDBLOCK�� ���� ó��, �۽ſ� Queue ����
			m_WritePD.PushIntoQueue( ulOffset );
			break;
		}
	}  // if( ( wSendSize != 0 ) && ( pbySendData != NULL ) )
	else  // �̼۽ŵǾ� Queue�� ���� Packet, ��ȯ �۽�
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
				// ���� �� �˻�
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

				// �۽�
				iResult = CNetwork::Sending( pbyToSendData, dwPacketLength );
				m_WritePD.UpdateMap( ulOffset, iResult );
				switch( iResult )
				{
				case NETWORK_IOC_STATE_SUCCESS:
					m_WritePD.RemoveFromMap( NETWORK_IOC_STATE_SUCCESS );
					break;

				case NETWORK_IOC_STATE_FAILURE:
					TLOG( LOG_DEBUG, _T( "[ERROR] The failed packet protocol is %d." ), *( ( WORD* )( pbyToSendData + 2 ) ) );  // UDP_HEADER ������ �ƴ� ��� Protocol
					// TODO: NETWORK_IOC_STATE_FAILURE�� ���,
					//		 CNetwork���� CNetwork::EndN()�� ȣ���Ͽ� ������ �����Ƿ�
					//		 �̿� ���õ� ������(ť,��,��Ÿ���)�� ��� ���ŵǾ�� �ϴµ�...
					//		 �Ʒ� m_WritePD.PopFromQueue()�� ���ؼ� �ٽ� �ѹ� �����غ���.
					// TODO: closesocket()�� ȣ��Ǿ��� ��, WSAEventSelect������ � �̺�Ʈ��
					//		 �߻��ϴ��� Ȯ���ؾ� �Ѵ�. FD_CLOSE�� �߻��Ѵٸ� FD_CLOSE ó������
					//		 ������ ��Ĺ�� ���� ��ü ���� �۾��� �ؾ� �ϰ� FD_CLOSE�� �߻�����
					//		 ������ �ٸ� ������ ������ ������ ��Ĺ�� ���� ��ü ���� �۾��� �ؾ� �Ѵ�.
					return NETWORK_IOC_STATE_FAILURE;

				case NETWORK_IOC_STATE_RETRY:  // WSAEWOULDBLOCK�� ���� ó��, ������ ��õ�
					return NETWORK_IOC_STATE_RETRY;
				}

				m_WritePD.PopFromQueue();

				// �ʱ�ȭ
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
			// Memory pool���� ���� ����
			bResult = m_ReadPD.BorrowFromMemoryPool( this, pbyUnitPacket, wREQDataSize, &ulOffset, &pbyStoredData );
			if( bResult == false || ulOffset == -1 || pbyStoredData == NULL )
			{
				TLOG( LOG_DEBUG, _T( "[ERROR] Failed to borrow the store space in memory pool." ) );
				bResult = false;
			}
			else
			{
				m_ReadPD.InsertIntoMap( ulOffset, NETWORK_IOC_STATE_DEFAULT );

				// Queue�� ����
				if( ThrowToMainQ( ulOffset ) )
					m_ReadPD.PushIntoQueue( ulOffset );  // TODO: ���� ť�� ����ϰ� �Ǹ� �� ó���� �ʿ� ����.
				// TODO: ThrowToMainQ()�� ��ȯ���� VOID�� �Ѵ�. ������, ���⼭�� bool�� �ϰ� �ִ�.
				//		 �̴� ����̸� �� ������ ���� ������ & ���� ť�� ����Ǹ� ThrowToMain()�� ��ȯ���� VOID�� �ϰ�
				//		 ThrowToMain()�� ȣ��ǹǷ� m_ReadPD.PushIntoQueue()�� ȣ���� �ʿ� ����.
				//		 ThrowToMain()�� ȣ���ϸ鼭 m_ReadPD.PushIntoQueue()�� ȣ���ϰ� �Ǹ�
				//		 CIOPacketDepository::m_queueWaitingToProcess�� ���ʿ��� �޸� Ȯ���� �߻��ϰ� �������� �ʴ´�.
				//		 ThrowToMainQ()�� ��ȯ���� bool�� �� ���� ThrowToMainQ()�� �����ǵ� ���������� m_ReadPD.PushIntoQueue()�� ȣ������ �ʱ� ���ؼ���.
				//		 ThrowToMainQ()�� �����ǵ� ������ ���� ������ & ���� ť�� ����� ������.
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

			// 1�� ����
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

			// 2�� ����
			n2ndLoadUpResult = LoadUpIntoQueue();
			if( n2ndLoadUpResult == 0 )  // ��ȯ �� -1�� 1�� ����
				return false;
			else if( n2ndLoadUpResult == -1 && nRemainingLength == -2 )  // ����! �� ��� ���� Loop
			{
				unsPacketLength = MAKEWORD( m_byReceivedDataBuffer[0], m_byReceivedDataBuffer[1] );

				TLOG( LOG_DEBUG, _T( "[ERROR] Failed to load up the packet unit into queue. Disconnecting with the opposite side(%#p)." ), this );
				TLOG( LOG_DEBUG,
					  _T( "          m_byReceivedDataBuffer : %#p, nPrevRemainingLength : %d, m_nRemainingLength : %d" ),
					  m_byReceivedDataBuffer, nPrevRemainingLength, m_nRemainingLength );
				TLOG( LOG_DEBUG,
					  _T( "          unsPrevPacketLength : %u, unsPacketLength : %u, nRepeatingCounter : %d" ),
					  unsPrevPacketLength, unsPacketLength, nRepeatingCounter );
				// NOTE: ȣ���� ������ ����� ���� ���� ó��
				return false;
			}
		}
		while( nRemainingLength == -2 );
		// ����! ���� Loop�� ������ �ʵ��� �����ؾ� �Ѵ�.
	}

	return true;
}

INT CPeer::LoadUpIntoQueue( VOID )
{
	while( true )
	{
		// 2�� ���� �غ�
		size_t unnTotalSizeOfRecvdBuffer = ( sizeof m_byReceivedDataBuffer );
		size_t unnUsingSize = ( unnTotalSizeOfRecvdBuffer - m_nRemainingLength );
		if( unnUsingSize < 2 )
			break;

		USHORT unsPacketLength = MAKEWORD( m_byReceivedDataBuffer[0], m_byReceivedDataBuffer[1] );
		if( unsPacketLength == 0 )  // ���� ����, ���� ���ŵ� Data�� ���� ���
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
			// NOTE: ȣ���� ������ ����� ���� ���� ó��

			// TODO: ���� pbyUnitPacket�� ���� �� ���� ũ���� ��Ŷ�� ���� ���̶��?
			//		 ��, ũ�⸸ Ŭ������ �������� ��Ŷ�̶�� ��� �ؾ��ұ�? ���⼭�� �̸� ��Ƴ� ������ ������ ó���� �� ����.
			//		 �׷��� ������ ���� ���� ������ �Ǵ� ũ�⸸ŭ ���ŵ� ��Ŷ �����͸� �����ϰ� ���� ��Ŷ���� ó���Ѵ�?
			return 0;
		}

		// 2�� ���� ����
		if( unsPacketLength <= unnUsingSize )
		{
			BYTE pbyUnitPacket[PACKET_BUFFER_SIZE] = {0,};
			bool bResult = false;
			ULONG_PTR ulOffset = -1;
			BYTE* pbyStoredData = NULL;
			SIZE_T unlSizeNeededToProcess = unnUsingSize - unsPacketLength;

			memcpy_s( pbyUnitPacket, PACKET_BUFFER_SIZE, m_byReceivedDataBuffer, unsPacketLength );

			// NOTE: ChangePacketProcessingMode()�� ���� ���� ���� �����Ͱ� ���ڱ� ���� �ٸ� ���� ���� �� �ִ�!
			//		 �̰��� ��Ŷ�� ���� ó���� �ϱ� �� ��Ŷ ó���� ��ᰡ �Ǵ� ���ŵ� �����͸� ���� ��Ŷ���� �����ϰų�
			//		 ���� ���۸� �����ϴ� ���̴�. �ٽ� ���� ���ŵ� Raw Data�� ó�� ���� ���̴�.
			//
			//		 Main Q�� Main Thread�� ������� �ʴ� ���� ��� ������ ����鼭 ������ ������ ChangePacket-
			//		 ProcessingMode()�� ���� ���ŵ� Raw Data�� ó�� ���� �̰����� ��Ŷ ó���� �̷����� �ƴ�.
			//		 �̷� ���� ���� ����� ���õ� Raw Data�� ó�� �߿� ChangePacketProcessingMode()���� ���� �����
			//		 ���õ� ��Ŷ�� ó���ϰ� �Ǹ� Raw Data ó���� ���� �������� �ʾҴµ� ������ �ʱ�ȭ �ع����� ������
			//		 �߻��� �� �ִ�. �̷� ��Ȳ�� '���� ó�� ����'�� ���� ��������� �߻��� �� �����Ƿ� �����ؾ� �Ѵ�.
			//		 �� Redmine #192 '���� ó��' ����
			if( false == ChangePacketProcessingMode( pbyUnitPacket ) )
			{
				// Memory pool���� ���� ����
				bResult = m_ReadPD.BorrowFromMemoryPool( this, pbyUnitPacket, unsPacketLength, &ulOffset, &pbyStoredData );
				if( bResult == false || ulOffset == -1 || pbyStoredData == NULL )
				{
					TLOG( LOG_DEBUG, _T( "[ERROR] Failed to borrow the store space in memory pool." ) );
					// NOTE: ȣ���� ������ ����� ���� ���� ó��
					return 0;
				}
				else
				{
					m_ReadPD.InsertIntoMap( ulOffset, NETWORK_IOC_STATE_DEFAULT );

					// Queue�� ����
					if( ThrowToMainQ( ulOffset ) )
						m_ReadPD.PushIntoQueue( ulOffset );  // TODO: ���� ť�� ����ϰ� �Ǹ� �� ó���� �ʿ� ����.
					// TODO: ThrowToMainQ()�� ��ȯ���� VOID�� �Ѵ�. ������, ���⼭�� bool�� �ϰ� �ִ�.
					//		 �̴� ����̸� �� ������ ���� ������ & ���� ť�� ����Ǹ� ThrowToMain()�� ��ȯ���� VOID�� �ϰ�
					//		 ThrowToMain()�� ȣ��ǹǷ� m_ReadPD.PushIntoQueue()�� ȣ���� �ʿ� ����.
					//		 ThrowToMain()�� ȣ���ϸ鼭 m_ReadPD.PushIntoQueue()�� ȣ���ϰ� �Ǹ�
					//		 CIOPacketDepository::m_queueWaitingToProcess�� ���ʿ��� �޸� Ȯ���� �߻��ϰ� �������� �ʴ´�.
					//		 ThrowToMainQ()�� ��ȯ���� bool�� �� ���� ThrowToMainQ()�� �����ǵ� ���������� m_ReadPD.PushIntoQueue()�� ȣ������ �ʱ� ���ؼ���.
					//		 ThrowToMainQ()�� �����ǵ� ������ ���� ������ & ���� ť�� ����� ������.
				}
			}

			// ������ 1�� ���縦 ���� Buffer ����
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
