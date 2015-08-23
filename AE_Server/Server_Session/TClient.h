#pragma once


class CClientTcp;

class TClient : public CPeer
{
public:
	TClient( VOID );
	virtual ~TClient( VOID ) {}

	inline VOID SetIsConnecting( bool bIsConnecting ) { SCOPED_LOCK( m_CS ) { m_bIsConnecting = bIsConnecting; } }
	inline CONST bool GetIsConnecting( VOID ) CONST { return m_bIsConnecting; }
	inline CONST U16 GetcreationNo( VOID ) CONST { return m_creationNo; }
	inline VOID SetGOID( U64 uni64GOID ) { SCOPED_LOCK( m_CS ){ m_GOID = uni64GOID; } }
	inline CONST U64 GetGOID( VOID ) { SCOPED_LOCK( m_CS ){ return m_GOID; } }
	inline VOID SetaffiliatedGameServerID( serverID_t affiliatedGameServerID ) { m_affiliatedGameServerID = affiliatedGameServerID; }
	inline CONST serverID_t GetaffiliatedGameServerID( VOID ) CONST { return m_affiliatedGameServerID; }

	bool InitAcpt( CClientTcp* pClientTcp, U16 creationNo, SOCKET ListenSocket );
	BOOL ReleaseAcpt( VOID );
	bool ReinitAcpt( SOCKET ListenSocket );
	VOID InduceSafeDisconnection( __in INT iHow = SD_RECEIVE );

	bool WritePacket( __in WORD wSendSize = 0, __in BYTE* pbySendData = NULL );

	bool OnIOConnected( VOID );
	VOID OnIODisconnected( VOID );
	VOID OnIORead( U32 dwDataLength );
	inline VOID OnIOWrote( VOID ) { WritePacket(); }

	virtual bool ChangePacketProcessingMode( __inout BYTE* pbyData );
	virtual bool ThrowToMainQ( __in ULONG_PTR ulReceivedData );
	bool InsertEOFDataIntoMainQ( VOID );

	void PacketProcess( ULONG_PTR ulProcessData );

private:
	CClientTcp* m_pClientTcp;
	volatile bool m_bIsConnecting;
	U16 m_creationNo;
	U64 m_GOID;
	serverID_t m_affiliatedGameServerID;

	bool m_bStopSend;
};
