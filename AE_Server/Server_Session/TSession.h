#pragma once


class CClientTcp;

class TSession : public CPeer
{
public:
	TSession( VOID );
	virtual ~TSession( VOID ) {}

	inline VOID SetClientTcp( CClientTcp* pClientTcp ) { m_pClientTcp = pClientTcp; }
	VOID SetIsConnecting( bool bIsConnecting ) { SCOPED_LOCK( m_CS ){ m_bIsConnecting = bIsConnecting; } }
	inline CONST bool GetIsConnecting( VOID ) CONST { return m_bIsConnecting; }
	inline VOID SetWaitingToDisconnect( bool bWaitingToDisconnect ) { m_bWaitingToDisconnect = bWaitingToDisconnect; }
	inline CONST bool GetWaitingToDisconnect( VOID ) CONST { return m_bWaitingToDisconnect; }
	inline VOID SetserverID( serverID_t serverIDValue ) { m_serverID.value = serverIDValue; }
	inline CONST serverID_t GetserverID( VOID ) CONST { return m_serverID.value; }

	bool Reinit( VOID );
	VOID Disconnect( VOID );  // Direct disconnection
	VOID InduceSafeDisconnection( VOID );

	bool OnIOConnected( VOID );
	VOID OnIODisconnected( VOID );  // Safe disconnection
	VOID OnIORead( U32 dwDataLength );
	VOID OnIOWrote( VOID );

	virtual bool ChangePacketProcessingMode( __in BYTE* pbyData );
	virtual bool ThrowToMainQ( __in ULONG_PTR ulReceivedData );

	void PacketProcess( ULONG_PTR ulProcessData );

private:
	CClientTcp* m_pClientTcp;
	volatile bool m_bIsConnecting;
	bool m_bWaitingToDisconnect;
	ServerID m_serverID;

	bool InsertEOFDataIntoMainQ( VOID );
};
