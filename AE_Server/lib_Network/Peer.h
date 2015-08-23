#pragma once

#include "./IOPacketDepository.h"


class CPeer : public CNetwork
{
public:
	CPeer( VOID ) {};
	CPeer( __in INT iNetworkKind ):CNetwork( iNetworkKind ) {};
	virtual ~CPeer( VOID ) {};

	virtual bool ChangePacketProcessingMode( __inout BYTE* pbyData ) { return false; };  // NOTE: 메인 큐와 메인 스레드에 의한 패킷 처리 방식이 아닌 다른 방식을 택할 경우 true 반환
	virtual bool ThrowToMainQ( __in ULONG_PTR ulReceivedData ) { return true; };  // NOTE: WSAEventSelect 모델을 사용하는 곳에서는 Main Q를 적용하지 않았다.

	BOOL StartP( VOID );
	BOOL EndP( VOID );

	bool ReadPacketForIOCP( __in DWORD dwReceiveSize = 0, __out BYTE* pbyReceivedPacket = NULL, __in ULONG_PTR ulProcessData = 0 );  // for IOCP
	bool ReadPacketForDOM( __in DWORD dwReceiveSize = 0 );  // for 'DOM'inion mode
	bool ReadPacketForWSAES( __out BYTE* pbyReceivedPacket = NULL );  // for 'WSAE'vent'S'elect
	INT WritePacket( __in WORD wSendSize = 0, __in BYTE* pbySendData = NULL );

protected:
	inline size_t GetSizeUnprocessedPacket( VOID ) { return m_ReadPD.GetSizemapTransceivingIOPacket(); }
	inline size_t GetSizeUnprocessedWritePD( VOID ) { return m_WritePD.GetSizemapTransceivingIOPacket(); }

	bool InsertREQDataIntoMainQ( __in WORD wREQDataSize, __in BYTE* pbyREQDataStartADR );

private:
	BYTE m_byReceivedDataBuffer[PACKET_BUFFER_SIZE * 3];
	volatile INT m_nRemainingLength;  // m_byReceivedDataBuffer에 빈 공간 크기

	CIOPacketDepository m_ReadPD;
	CIOPacketDepository m_WritePD;

	bool IntegratedLoadForRawData( VOID );
	INT LoadUpIntoQueue( VOID );
};
