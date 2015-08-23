#pragma once


namespace SERVER_INFO
{
	typedef struct tagServerBaseInfo
	{
		std::tstring name;
		NetAddress addr;
	} sServerBaseInfo;

	struct sPublicInfoPack
	{
		sServerBaseInfo PrototypeS;
	};
}
