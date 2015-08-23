#pragma once

#include "./Define_Network.h"

#define NETWORK_IOC_STATE_DEFAULT 2
#define NETWORK_IOC_STATE_SUCCESS 1
#define NETWORK_IOC_STATE_PENDING 0
#define NETWORK_IOC_STATE_FAILURE -1
#define NETWORK_IOC_STATE_RETRY -2

#define NETWORK_KIND_DEFAULT 0
#define NETWORK_KIND_SERVER 1
#define NETWORK_KIND_CLIENT 2
#define NETWORK_KIND_LISTEN_SERVER 3
#define NETWORK_KIND_LISTEN_CLIENT 4


class CNetwork
{
public:
	CNetwork( __in INT iNetworkKind = NETWORK_KIND_DEFAULT );
	virtual ~CNetwork( VOID ) {};

	BOOL CreateNBOLSocket( VOID );  // 'N'on-'B'locking 'O'ver'L'apped
	bool CreateConnection( __in LPSTR lpszAddr, __in USHORT unsPort );

	inline TCriticalSection* GetCS( VOID ) { return &m_CS; }
	inline CONST INT GetNetworkKind( VOID ) CONST { return m_iNetworkKind; }
	inline SOCKET GetNSocket( VOID ) { SCOPED_LOCK( m_CS ){ return m_uni64NSocket; } }

	BOOL StartN( VOID );
	BOOL EndN( VOID );

	BOOL Listen( __in USHORT port, __in INT backLog );
	BOOL Accept( __in SOCKET listenSocket );  // TODO: NPC 서버의 패킷 처리 부분 적용이 완료되면 이 함수에 대한 동기화 및 문제점은 없는지 재확인해야 한다.
	bool HalfClose( __in INT iHow );

protected:
	TCriticalSection m_CS;
	TCriticalSection m_CSForReceiving;
	TCriticalSection m_CSForSending;

	DWORD m_dwNumberOfBytesRecvd;
	bool m_bWasPendingRecv;
	bool m_bIsSD_RECEIVE;
	bool m_bIsSD_SEND;

	inline OVERLAPPED_EX& GetstReceiveOverlappedEX( VOID ) { return m_stReceiveOverlappedEX; }

	INT Receiving( bool bIsWSAES = false );
	INT Sending( __in BYTE* pbyData, __in DWORD dwDataLength );

	VOID UpdateResultOfOverlappedRecv( VOID );
	INT QueuingReceivedData( __in BYTE* pbyEmptySection, __in INT nRemainingLength );

private:
	INT m_iNetworkKind;
	SOCKET m_uni64NSocket;

	BYTE m_byReceiveBuffer[PACKET_BUFFER_SIZE];

	OVERLAPPED_EX m_stAcceptOverlappedEX;
	OVERLAPPED_EX m_stReceiveOverlappedEX;
	OVERLAPPED_EX m_stSendOverlappedEX;
};
