#pragma once

#include "./PacketHandlerMGR.h"
#include "./TSession.h"
#include "./TClient.h"


extern volatile LONG g_CCU;

class CClientTcp : public CIocpForNetwork
{
	typedef std::list<TSession*> TSessionList;
	typedef TSessionList::iterator itorTSessionList;

public:
	CClientTcp( VOID );
	~CClientTcp( VOID ) {}

	inline bool RegisterSocketToIocp( SOCKET socket, ULONG_PTR completionKey ) { return CIocpForNetwork::RegisterSocketToIocp( socket, completionKey ); }
	inline bool CloseWorkThreadAndIOCP( VOID ) { return CIocpForNetwork::End(); }
	inline CPacketHandlerMGR& GetPacketHandlerMGR( VOID ) { return m_packetHandlerMGR; }
	inline CONST U32 GetConcurrentServer( VOID ) CONST { return m_pmapConnectedServerList->Count(); }
	inline VOID AddConnectedClient( objectHandle_t GOID, TClient* pConnectedClient ) { m_pmapConnectedClientList->Add( GOID, pConnectedClient ); }
	inline VOID RemoveConnectedClient( objectHandle_t GOID ) { m_pmapConnectedClientList->Remove( GOID ); }
	inline U32 GetCCU( VOID ) { return m_pmapConnectedClientList->Count(); }
	inline VOID SetpWorldServer( TSession* pWorldServer ) { m_pWorldServer = pWorldServer; }
	inline CONST serverID_t GetWorldServerID( VOID ) CONST { return ( m_pWorldServer? ( m_pWorldServer->GetserverID() ) : 0 ); }

	bool Begin( VOID );
	VOID End( VOID );

	SOCKET GetListenSocket( VOID );
	bool EndpListen( VOID );
	bool RebirthOfListenSocket( VOID );
	bool CreateServerConnection( NetAddress& rConnectionAddr );
	VOID DisconnectedServer( TSession* pServer );
	VOID ReusableSession( TSession* pReusableSession );
	VOID FindConnectedClient( objectHandle_t GOID, TClient** ppClient );
	bool SendToServer( serverID_t toDestServerID, TSPacket& rspkPack );
	VOID ServerSessShutdown( bool bIsWholeServerSess = true );
	VOID ClientSessShutdown( VOID );

	VOID UpdateCCU( VOID );
	VOID DisplayConnectedServerNetworkInfo( VOID );

protected:
	VOID OnIOConnected( VOID* pObject );
	VOID OnIODisconnected( VOID* pObject, bool bIsForcedDisconnection = false );
	VOID OnIORead( VOID* pObject, U32 dwDataLength );
	VOID OnIOWrote( VOID* pObject );

	VOID StartShutdown( VOID );

private:
	TCriticalSection m_CS;

	CPacketHandlerMGR m_packetHandlerMGR;
	CNetwork* m_pListen;
	bool m_bIsRebirthListen;

	TFixedMemoryPool<TSession, MAX_PEER_IN_PROTOTYPESERVER>* m_pInactiveTServerElem;
	TMap<ULONG_PTR, TSession*, NumberHash<ULONG_PTR>>* m_pActiveTServerElem;
	TSessionList m_listUsableTServerElem;  // 상대방에게 능동적으로 연결을 시도하는 객체를 원소로 갖는 재활용 목록
	TMap<serverID_t, TSession*, NumberHash<serverID_t>>* m_pmapConnectedServerList;

	TFixedMemoryPool<TClient, MAX_USER_IN_SESSIONSERVER>* m_pInactiveTClientElem;
	TMap<ULONG_PTR, TClient*, NumberHash<ULONG_PTR>>* m_pActiveTClientElem;
	TMap<objectHandle_t, TClient*, NumberHash<objectHandle_t>>* m_pmapConnectedClientList;

	TSession* m_pWorldServer;

	TSession* GetTServerFromPool( VOID );
	bool ReturnTServerToPool( TSession* pTServer );
	VOID ReleaseServerConnection( VOID );
	bool CreateClientConnection( SOCKET ListenSocket );
	TClient* GetTClientFromPool( VOID );
	bool ReturnTClientToPool( TClient* pTClient );
	VOID ReleaseClientConnection( VOID );
	bool BreakListenSocket( VOID );
};
