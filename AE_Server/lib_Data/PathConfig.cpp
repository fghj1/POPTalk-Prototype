#include "DataPCH.h"
#include "PathConfig.h"
#include "./GameConfig.h"


bool PathConfig::loadConfigFile()
{
	std::string strConfigFile = W2M( ( CONFIG()->getCommonPath() ) + _T("Config\\PathConfig.xml") ).c_str();
	TiXmlDocument ConfigInfoXML( strConfigFile );

	if( ConfigInfoXML.LoadFile() == false )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to load the XML file of next path.\n(%s)" ), M2W( strConfigFile.c_str() ).c_str() );
		return false;
	}

	// 'PathConfig' part
	TiXmlElement* pRoot = ConfigInfoXML.FirstChildElement( "PathConfig" );
	if( pRoot == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Not found an element that matches a specified string(%s)." ), _T( "PathConfig" ) );
		return false;
	}

	// file_DB
	TiXmlElement* pElement = pRoot->FirstChildElement( "file_DB" );
	if( pElement == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Not found an element that matches a specified string(%s)." ), _T( "file_DB" ) );
		return false;
	}
	m_fileDBName = M2W( pElement->Attribute( "fileName" ) ).c_str();

	// Stat_DB
	pElement = pRoot->FirstChildElement( "Stat_DB" );
	if( pElement == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Not found an element that matches a specified string(%s)." ), _T( "Stat_DB" ) );
		return false;
	}
	m_statDBName = M2W( pElement->Attribute( "fileName" ) ).c_str();

	// Character_DB
	pElement = pRoot->FirstChildElement( "Character_DB" );
	if( pElement == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Not found an element that matches a specified string(%s)." ), _T( "Character_DB" ) );
		return false;
	}
	m_charDBName = M2W( pElement->Attribute( "fileName" ) ).c_str();

	// Monster_DB
	pElement = pRoot->FirstChildElement( "Monster_DB" );
	if( pElement == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Not found an element that matches a specified string(%s)." ), _T( "Monster_DB" ) );
		return false;
	}
	m_monsterDBName = M2W( pElement->Attribute( "fileName" ) ).c_str();

	return true;
}
