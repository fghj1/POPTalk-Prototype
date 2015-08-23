#pragma once

#include "./ClientTcp.h"
#include "./TClient.h"
#include "./TSession.h"


class CSessionServer : public TSingleton<CSessionServer>
{
	struct UnitNode
	{
		bool bIsClient;
		LPVOID pHostOfReceivedData;
		ULONG_PTR ulReceivedData;

		UnitNode() : bIsClient( true ), pHostOfReceivedData( NULL ), ulReceivedData( 0 ) {}
	};

public:
	CSessionServer( VOID );
	~CSessionServer( VOID ) {}

	bool Begin( VOID );
	VOID DisplayConnectedServerNetworkInfo( VOID );
	VOID RebirthOfListenSocket( VOID );
	VOID GetSizeSessionSMainQ( VOID );
	VOID ChangeUpdateCycle( VOID );
	UINT SessionSMainThread( VOID );
	VOID InsertIntoMainQ( bool bIsClient, LPVOID pHostOfReceivedData, ULONG_PTR ulReceivedData );
	VOID ShutdownSequence( VOID );

	inline VOID SetserverID( serverID_t serverIDValue ) { m_serverID.value = serverIDValue; }
	inline CONST serverID_t GetserverID( VOID ) CONST { return m_serverID.value; }
	inline CONST TAuto_Ptr<CClientTcp> GetTcpClient( VOID ) CONST { return m_pTcpClient; }
	inline VOID SetbFinishServer( BOOL bFinishServer ) { InterlockedExchange( ( LONG volatile* )&m_bFinishServer, ( LONG )bFinishServer ); }
	inline CONST volatile BOOL GetbFinishServer( VOID ) CONST { return m_bFinishServer; }
	inline CONST volatile bool GetbMainThreadFinished( VOID ) CONST { return m_bMainThreadFinished; }

private:
	ServerID m_serverID;
	CLockFreeQueue<UnitNode> m_SessionSMainQ;
	TAuto_Ptr<CClientTcp> m_pTcpClient;
	UINT m_unnProcTick;
	HANDLE m_hSessionSMainThread;
	volatile BOOL m_bFinishServer;
	UINT m_nDelayCounter;
	UINT m_nRepeatCounter;
	eShutdownPhase m_eNextShutdownPhase;
	volatile bool m_bFinishMainThread;
	volatile bool m_bMainThreadFinished;
};

inline static CSessionServer* SER()
{
	return CSessionServer::Instance();
}
