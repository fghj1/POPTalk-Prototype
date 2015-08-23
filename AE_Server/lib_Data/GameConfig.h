#pragma once

#include "./ServerInfo.h"

#define CONFIGINFO_PATH _T( "Config\\ServerConfig.xml" )


class CGameConfig :	public TSingleton<CGameConfig>
{
public:
	CGameConfig() {};
	~CGameConfig() {};

	inline sCHAR GetServerName( VOID ) CONST { return m_ServerName; }
	inline sCHAR getRootPath( VOID ) CONST { return m_ServerPath; }
	inline sCHAR getCommonPath( VOID ) CONST { return m_CommonPath; }
	inline SERVER_INFO::sPublicInfoPack GetPubServerInfo( VOID ) { return m_PubServerInfo; };
	inline NetAddress GetlocalAddr( VOID ) CONST { return m_localAddr; }

	bool InitEnvironmentConfig( TCHAR* pszServerName, eServerType eType );
	NetAddress* GetPubServerADR( U8 nServeType );

private:
	eServerType m_ServerType;
	U8 m_ServerId;
	sCHAR m_ServerName;
	sCHAR m_ServerPath;
	sCHAR m_CommonPath;
	SERVER_INFO::sPublicInfoPack m_PubServerInfo;
	NetAddress m_localAddr;

	bool setRootPath( VOID );
	bool loadConfigFile( VOID );
	VOID DisplayServerInfo( VOID );
};

inline static CGameConfig* CONFIG()
{
	return CGameConfig::Instance();
}
