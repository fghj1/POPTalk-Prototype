//-----------------------------------------------------------------
//						AE_ServerDefine.h
//-----------------------------------------------------------------

#ifndef __AE_SERVER_DEFINE__H__
#define __AE_SERVER_DEFINE__H__

#define SESSION_PORT_FOR_CLIENT 8035
#define MAX_USER_IN_SESSIONSERVER 100
#define MAX_PEER_IN_PROTOTYPESERVER 1

#pragma pack( push, 1 )

typedef enum tagServerType
{
	SERVER_NONE = 0,
	SERVER_PROTOTYPE,
	SERVER_END,
} eServerType;

typedef enum tagShutdownPhase
{
	SHUTDOWNPHASE_START,
	BLOCK_NEWCONNECTION,  // ���ο� ���� ����
	HALFCLOSE_CLIENT,  // Ŭ���̾�Ʈ �ڿ� ����
	HALFCLOSE_SERVER,  // Ÿ ���� �ڿ� ����
	FINISH_WORKTHREADIOCP_NETWORK,  // ��Ʈ��ũ ó���� ���� IOCP�� �۾� ������ ����� IOCP ���
	FINISH_MAINTHREAD,  // ���� ������ ����
	FINISH_WORKTHREADIOCP_THREADPOOL,  // ������ Ǯ ����� ���� IOCP�� �۾� ������ ����� IOCP ���
	RELEASE_RESOURCE,  // ���� �ڿ� ����
	FINISH_ENTRYPOINTTHREAD,  // main() �Լ� ������ ����
	SHUTDOWNPHASE_END
} eShutdownPhase;

typedef enum tagRETURN_CODE
{	
	LP_ERR		= -1,		// ���� 
	LP_NONE		=  0,		// ���� ���� 
	LP_OK		=  1,		// ���� 
	LP_CHECK	=  2,
	LP_REMAIN	=  3,
	LP_FULL		=  4,
	LP_RETURN_END,		
} LP_RETURN;

typedef union tagServerID
{
	struct tagDetail
	{
		U16 m_classification;
		U16 m_startupNo;
	};

	struct tagDetail items;
	serverID_t value;

	union tagServerID( U16 classification = 0, U16 startupNo = 0 )
	{
		items.m_classification = classification;  // ���� ���� �ĺ���
		items.m_startupNo = startupNo;  // ��ü ���� ��ȣ
	}
} ServerID;

//----------------------------------------------------
// ������ ������ Ű ���� 
typedef union tagUID
{
	struct tagUDI
	{
		U32		nTime;		// ���� �ð� 
		U16		nMap;		// ������ �� �ѹ� 
		U8		nSession;	// ���� ���� �ѹ� 
		U8		nSG;		// ���� �� �ѹ� 
	};

	struct tagUDI		sInf;				
	U64					dwKey;	

	union tagUID(U8 nG, U8 nS, U16 nM)	
	{
		DWORD dwTime = timeGetTime();

		sInf.nSG		= nG; 
		sInf.nSession	= nS; 
		sInf.nMap		= nM; 
		sInf.nTime      = (U32)dwTime; 
	}

	union tagUID(const U64 nKey)	
	{ 
		dwKey = nKey;
	}

	bool operator==(const U64 key)
	{
		if (dwKey == key)		return true;
		else					return false;
	}

	U64& operator = ( union tagUID gK )
	{
		return dwKey;
	}

} UID;

#pragma pack(pop)

#endif 