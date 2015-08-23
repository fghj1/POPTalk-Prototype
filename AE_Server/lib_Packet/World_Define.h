//---------------------------------------------------------------------
//							World_Define.h
//---------------------------------------------------------------------


#ifndef __WORLD_DEFINE__H__
#define __WORLD_DEFINE__H__

#include "../lib_Common/CommonPCH.h"		

#pragma	pack( push, 1 )



//-----------------------------------------------------------------
// Npc Key ���� �ѹ� 
typedef union tagNpcCata
{
	struct itClass
	{
		U16		number;		// Npc Ÿ�� 
		U16		type;		// Npc Sub Type 
	};

	struct itClass		Class;				
	U32					dwClass;		// �ѹ���
} NpcCata;


typedef union tagNpcID
{
	struct NpcKey
	{
		NpcCata			Cate;
		U32				nTime;		// ���� �ð� 
	};
	
	struct NpcKey		Key;
	U64					dwSerial;

	union tagNpcID()
	{}

	union tagNpcID(U16 nN, U16 nT)	
	{
		DWORD dwTime = timeGetTime();

		Key.Cate.Class.number		= nN; 
		Key.Cate.Class.type	= nT; 
		Key.nTime      = (U32)dwTime; 
	}

} NpcID;		



//----------------------------------------------------
// ���� �ڵ� 
typedef union tagLP_ERROR
{
	struct tagRT
	{
		S8		rt;						// ���� �ڵ� 
		U8		type;					// Ÿ�� �ڵ� 
		U16		errCode;				// ���� �ڵ� 
	};

	struct tagRT		sCode;				
	U32					nNum;	

	union tagLP_ERROR(LP_RETURN rt_code = LP_OK)	{ sCode.rt = rt_code; }

	bool operator==(const LP_RETURN code)
	{
		if (sCode.rt == code)		return true;
		else						return false;
	}
	
} LP_ERROR;		


//-----------------------------------------------------------------
// ������ ī�װ� �з��� ���Ե� �ѹ� 
//  U32 nTot =  (nMain*BIT(24)) + (nSub*BIT(16)) + (nType);
typedef union tagCategory
{
	struct itClass
	{
		U16		type;		// Ÿ��
		U8		sub;		// ���� ī�װ� 
		U8		main;		// ���� ī�װ� 
	};

	struct itClass		Class;				
	U32					dwClass;		// �ѹ���
} itCate;



typedef union tagItemIndex
{
	struct itIndex
	{
		itCate			Cate;
		U32				Cnt;		// �ѹ���
	};
	
	struct itIndex		Index;
	U64					dwSerial;
} ItemIndex;

// ������ ��Ϲ�ȣ
typedef union tagItemRegistNumber
{
	struct itRegNum{
		DWORD	second		: 6;
		DWORD	minute		: 6;
		DWORD	hour		: 5;
		DWORD	day			: 5;
		DWORD	month		: 4;
		DWORD	year		: 6;	// 6+6+5+5+4+6 = 32bit
		
		DWORD	zone		: 12;
		DWORD	channel		: 5;
		DWORD	server		: 7;
		DWORD	world		: 4;
		DWORD	method		: 4;	// 12+5+7+4+4 = 32bit

		DWORD	sector		: 12;
		DWORD	serial		: 10;
		DWORD	notuse		: 10;	// 12+10+10 = 32bit
	};
	
	struct itRegNum		RegNum;
	DWORD			dwSerial[3];

	union tagItemRegistNumber()
	{
		ZeroMemory(&RegNum, sizeof(itRegNum));
	}
} ItemRegNum;


typedef union tagObjectKey
{
	struct obKey
	{
		U32				Cnt;		// �ѹ���
		itCate			Cate;
	};
	
	struct obKey		Key;
	U64					dwSerial;	
} OBKEY;		


//-----------------------------------------------------------------
// Zone ���� 
#define MAP_X_MIN_SIZE 0
#define MAP_X_MAX_SIZE 4096
#define MAP_Y_MIN_SIZE 0
#define MAP_Y_MAX_SIZE 4096
#define MAP_Z_MIN_SIZE 0
#define MAP_Z_MAX_SIZE 4096

typedef union tagZoneID
{
	struct Composition
	{
		WORD wMapID;  // �� �ĺ���ȣ
		WORD wChannelNo;  // ä�ι�ȣ
	};

	Composition stUnit;
	DWORD dwValue;  // �� �ĺ���ȣ

	tagZoneID()
	{
		dwValue = 0;
	}
} uiZoneID;


// ������Ʈ Ÿ�� 
typedef enum tagCollidType
{
	COLL_PLAN			= 0,
	COLL_BOX			= 1,		// BOX Ÿ�� ������Ʈ 
	COLL_CIRCLE			= 2,		// ���� 
	COLL_END,

} eCollidType;

// Ÿ�� �̺�Ʈ ���� 
typedef struct tagEventHandle
{
	U16				type;					// �޽��� Ÿ�� 
	U16				msgid;					// �޽��� ���̵� 	
	U64				senderUid;				// ������ ��� ���̵� 
	U64				recverUid;				// ������� ���̵� 
} EventHandle;

// NPC ��ü ���ű��� ��� �ð�
#define WAIT_REMOVE_DEAD		300000	// 5��
#define	WAIT_REMOVE_DEAD_LOOT	60000	// 60��

#pragma pack(pop)

#endif 