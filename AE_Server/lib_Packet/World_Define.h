//---------------------------------------------------------------------
//							World_Define.h
//---------------------------------------------------------------------


#ifndef __WORLD_DEFINE__H__
#define __WORLD_DEFINE__H__

#include "../lib_Common/CommonPCH.h"		

#pragma	pack( push, 1 )



//-----------------------------------------------------------------
// Npc Key 생성 넘버 
typedef union tagNpcCata
{
	struct itClass
	{
		U16		number;		// Npc 타입 
		U16		type;		// Npc Sub Type 
	};

	struct itClass		Class;				
	U32					dwClass;		// 넘버링
} NpcCata;


typedef union tagNpcID
{
	struct NpcKey
	{
		NpcCata			Cate;
		U32				nTime;		// 생성 시간 
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
// 에러 코드 
typedef union tagLP_ERROR
{
	struct tagRT
	{
		S8		rt;						// 리턴 코드 
		U8		type;					// 타입 코드 
		U16		errCode;				// 에러 코드 
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
// 아이템 카테고리 분류가 포함된 넘버 
//  U32 nTot =  (nMain*BIT(24)) + (nSub*BIT(16)) + (nType);
typedef union tagCategory
{
	struct itClass
	{
		U16		type;		// 타입
		U8		sub;		// 하위 카테고리 
		U8		main;		// 메인 카테고리 
	};

	struct itClass		Class;				
	U32					dwClass;		// 넘버링
} itCate;



typedef union tagItemIndex
{
	struct itIndex
	{
		itCate			Cate;
		U32				Cnt;		// 넘버링
	};
	
	struct itIndex		Index;
	U64					dwSerial;
} ItemIndex;

// 아이템 등록번호
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
		U32				Cnt;		// 넘버링
		itCate			Cate;
	};
	
	struct obKey		Key;
	U64					dwSerial;	
} OBKEY;		


//-----------------------------------------------------------------
// Zone 구성 
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
		WORD wMapID;  // 맵 식별번호
		WORD wChannelNo;  // 채널번호
	};

	Composition stUnit;
	DWORD dwValue;  // 존 식별번호

	tagZoneID()
	{
		dwValue = 0;
	}
} uiZoneID;


// 오브젝트 타입 
typedef enum tagCollidType
{
	COLL_PLAN			= 0,
	COLL_BOX			= 1,		// BOX 타입 오브젝트 
	COLL_CIRCLE			= 2,		// 원형 
	COLL_END,

} eCollidType;

// 타임 이벤트 관련 
typedef struct tagEventHandle
{
	U16				type;					// 메시지 타입 
	U16				msgid;					// 메시지 아이디 	
	U64				senderUid;				// 보내는 사람 아이디 
	U64				recverUid;				// 받을사람 아이디 
} EventHandle;

// NPC 사체 제거까지 대기 시간
#define WAIT_REMOVE_DEAD		300000	// 5분
#define	WAIT_REMOVE_DEAD_LOOT	60000	// 60초

#pragma pack(pop)

#endif 