#pragma once


class PathConfig : public TSingleton<PathConfig>
{
public:
	PathConfig() {};
	~PathConfig() {};

public:
	bool	loadConfigFile();

	sCHAR		getfileDB_Name()	{	return m_fileDBName; };
	sCHAR		getstatDB_Name()	{	return m_statDBName; };
	sCHAR		getCharacterDB_Name(){	return m_charDBName; };
	sCHAR		getMonsterDB_Name(){return m_monsterDBName;}
private:
	sCHAR				m_fileDBName;
	sCHAR				m_statDBName;
	sCHAR				m_charDBName;
	sCHAR				m_monsterDBName;
};

inline static PathConfig* PATH()
{
	return PathConfig::Instance();
}
