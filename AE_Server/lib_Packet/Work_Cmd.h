//------------------------------------------------------------------
//							DB_Cmd.h
//------------------------------------------------------------------

#ifndef __DBCMD__H__
#define __DBCMD__H__


typedef enum tagDBCMD
{
	DBCMD_NONE_NONE		= 0,
	DBCMD_BASE_START	= 1,

	DBCMD_UPDATE_CHARACTERINFO,
	DBCMD_ITEM_ADDOBJECT,
	DBCMD_ITEM_SAVE,
	DBCMD_AUTH_ADDITEM,

	

	DBCMD_END
} DBCMD;


typedef enum tagPKCMD
{
	PKCMD_BASE_START= DBCMD_END,

	PKCMD_SEND_SERVER,
	PKCMD_SEND_ITEMLIST,
	PKCMD_SEND_UNREGISTER,

	PKCMD_END
} PKCMD;

#endif 
