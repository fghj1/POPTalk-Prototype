#include "DataPCH.h"
#include "ServerInfo.h"
#include "GameConfig.h"
#include <regex>

#ifdef GET_PROC_NAME
#include <Psapi.h>

#pragma comment( lib, "psapi.lib" )
#endif


bool CGameConfig::InitEnvironmentConfig( TCHAR* pszServerName, eServerType eType )
{
	if( pszServerName == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] A first parameter(%#p) is invalid." ), pszServerName );
		return false;
	}

	m_ServerName = pszServerName;

#ifdef GET_PROC_NAME
	DWORD dwPID = GetCurrentProcessId();
	TCHAR szProcessName[256] = _T( "<unknown>" );
	HANDLE hProc = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPID );

	if( hProc != NULL )
	{
		HMODULE hMod;
		DWORD cbNeeded;
		if( EnumProcessModules( hProc, &hMod, ( sizeof hMod ), &cbNeeded ) )
			GetModuleBaseName( hProc, hMod, szProcessName, ( sizeof szProcessName ) / sizeof( TCHAR ) );
	}
	LOGF()->setApp( szProcessName, dwPID );
#else
	LOGF()->setApp( pszServerName, GetCurrentProcessId() );
#endif
	m_ServerType = eType;

	if( setRootPath() == false )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Function failed." ) );
		return false;
	}

	if( loadConfigFile() == false )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to load config file." ) );
		return false;
	}

	DisplayServerInfo();

	return true;
}

NetAddress* CGameConfig::GetPubServerADR( U8 nServeType )
{
	NetAddress* pAddress = NULL;

	switch( nServeType )
	{
	case SERVER_PROTOTYPE:
		pAddress = &m_PubServerInfo.PrototypeS.addr;
		break;
	}

	return pAddress;
};

bool CGameConfig::setRootPath( VOID )
{
	HMODULE hModule;
	TCHAR szTemp[MAX_PATH] = {0,};
	sCHAR strFilePath;
	DWORD dwResult;
	INT nIndex = 0;

	// 실행중인 Server process의 Handle
	hModule = GetModuleHandle( NULL );
	if( hModule == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to extract the module handle." ) );
		strFilePath.Empty();
		return false;
	}

	// 실행된 Server의 경로 추출
	dwResult = GetModuleFileName( hModule, szTemp, MAX_PATH );
	if( dwResult == 0 )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to extract the module name." ) );
		strFilePath.Empty();
		return false;
	}
	strFilePath = szTemp;

	// 플랫폼 종속적인 경로 지정
	nIndex = strFilePath.findLast( _T( '\\' ) );
	if( nIndex == -1 )
	{
		nIndex = strFilePath.findLast( _T( '/' ) );
		if( nIndex == -1 )
		{
			TLOG( LOG_DEBUG, _T( "[ERROR] Failed to find the '/'." ) );
			strFilePath.Empty();
			return false;
		}
	}
	strFilePath.replace( ( U32 )( nIndex + 1 ), _T( '\0' ) );

	m_ServerPath = strFilePath;  // Server의 실행 경로 반환

	// 플랫폼 독립적인 경로 지정
	INT nDepth = 0;
	for( nDepth = -2; nDepth < 0; ++nDepth )
	{
		nIndex = strFilePath.findLast( _T( '\\' ) );
		if( nIndex == -1 )
		{
			nIndex = strFilePath.findLast( _T( '/' ) );
			if( nIndex == -1 )
			{
				TLOG( LOG_DEBUG, _T( "[ERROR] Failed to find the '/'." ) );
				strFilePath.Empty();
				return false;
			}
		}

		strFilePath.replace( ( U32 )nIndex, _T( '\0' ) );
	}
	strFilePath += _T( '\\' );

	m_CommonPath = strFilePath;

	return true;
}

bool CGameConfig::loadConfigFile( VOID )
{
	// 서버 공통 설정 정보 추출
	std::string strConfigFile = W2M( m_CommonPath + CONFIGINFO_PATH ).c_str();
	TiXmlDocument ConfigInfoXML( strConfigFile );

	if( ConfigInfoXML.LoadFile() == false )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to load the XML file of next path.\n(%s)" ), M2W( strConfigFile.c_str() ).c_str() );
		return false;
	}

	// 'ServerConfig'
	TiXmlElement* pRoot = ConfigInfoXML.FirstChildElement( "ServerConfig" );
	if( pRoot == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Not found an element that matches a specified string(%s)." ), _T( "ServerConfig" ) );
		return false;
	}
	
	// 'Server' part
	TiXmlElement* pServerRoot = pRoot->FirstChildElement( "Server" );
	if( pServerRoot == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Not found an element that matches a specified string(%s)." ), _T( "Server" ) );
		return false;
	}

	// Prototype Server
	TiXmlElement* pElement = pServerRoot->FirstChildElement( "Prototype" );
	if( pElement == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Not found an element that matches a specified string(%s)." ), _T( "Prototype" ) );
		return false;
	}
	m_PubServerInfo.PrototypeS.name = M2W( pElement->Attribute( "name" ) ).c_str();
	if( m_PubServerInfo.PrototypeS.addr.SetStringAddress( ( CHAR* )( pElement->Attribute( "ip" ) ) ) == false )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Not found an attribute that matches a specified string(%s)." ), _T( "ip" ) );
		return false;
	}
	m_PubServerInfo.PrototypeS.addr.port = ( WORD )( atoi( pElement->Attribute( "port" ) ) );

	return true;
}

VOID CGameConfig::DisplayServerInfo( VOID )
{
	CHAR szLocalHostName[256] = {0,};

	TLOG( LOG_CRT, _T( "The network information of %s" ), m_ServerName.c_str() );

	INT nResult = gethostname( szLocalHostName, ( sizeof szLocalHostName ) );
	if( nResult != SOCKET_ERROR )
	{
#ifdef _UNICODE
		TLOG( LOG_CRT, _T( "Local host name : %s"), M2W( szLocalHostName ).c_str() );
#else
		TLOG( LOG_CRT, _T( "Local host name : %s"), szLocalHostName );
#endif

		PHOSTENT pstHostInfo = gethostbyname( szLocalHostName );
		if( pstHostInfo != NULL )
		{
			INT nIndex = 0;
			IN_ADDR stIP;

			while( pstHostInfo->h_addr_list[nIndex] != NULL )
			{
				stIP.S_un.S_addr = *( ( PULONG )( pstHostInfo->h_addr_list[nIndex++] ) );
#ifdef _UNICODE
				TLOG( LOG_CRT, _T( "Local host IP : %s"), M2W( inet_ntoa( stIP ) ).c_str() );
#else
				TLOG( LOG_CRT, _T( "Local host IP : %s"), inet_ntoa( stIP ) );
#endif
			}

			if( pstHostInfo->h_addr_list[0] != NULL )
				m_localAddr.ip = *( ( PULONG )( pstHostInfo->h_addr_list[0] ) );
		}
	}

	NetAddress* pNetADR = GetPubServerADR( m_ServerType );
	if( pNetADR != NULL )
		m_localAddr.port = pNetADR->port;
	TLOG( LOG_CRT, _T( "Local host port : %d" ), m_localAddr.port );
}
