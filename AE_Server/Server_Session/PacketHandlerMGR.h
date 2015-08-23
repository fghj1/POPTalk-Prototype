#pragma once

#include "./PacketHandlerForServer.h"
#include "./PacketHandlerForClient.h"


class CPacketHandlerMGR
{
public:
	CPacketHandlerMGR( VOID );
	~CPacketHandlerMGR( VOID ) {}

	bool InitPacketHandlerMGR();
	VOID ReleasePacketHandlerMGR();

	//bool IsRegdProtocol( U16 nProtocol );
	tPacketHandler GetHandler( U16 nProtocol );
	eServerType GetDestination( U16 nProtocol );

private:
	//TIndex m_RegdProtocol;
	TMap<DWORD, tPacketHandler, NumberHash<DWORD>>* m_pmapHandlers;
	CPacketHandlerForServer m_handlerDefForServer;
	CPacketHandlerForClient m_handlerDefForClient;
	CAtlMap<int, eServerType> m_mapProtocolToAffiliatedServer;

	VOID AddHandler( U16 nProtocol, tPacketHandler& rHandler );
};
