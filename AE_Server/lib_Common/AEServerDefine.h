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
	BLOCK_NEWCONNECTION,  // 새로운 연결 차단
	HALFCLOSE_CLIENT,  // 클라이언트 자원 정리
	HALFCLOSE_SERVER,  // 타 서버 자원 정리
	FINISH_WORKTHREADIOCP_NETWORK,  // 네트워크 처리를 위한 IOCP의 작업 스레드 종료와 IOCP 폐쇄
	FINISH_MAINTHREAD,  // 메인 스레드 종료
	FINISH_WORKTHREADIOCP_THREADPOOL,  // 스레드 풀 운용을 위한 IOCP의 작업 스레드 종료와 IOCP 폐쇄
	RELEASE_RESOURCE,  // 각종 자원 해제
	FINISH_ENTRYPOINTTHREAD,  // main() 함수 스레드 종료
	SHUTDOWNPHASE_END
} eShutdownPhase;

typedef enum tagRETURN_CODE
{	
	LP_ERR		= -1,		// 실패 
	LP_NONE		=  0,		// 설정 없음 
	LP_OK		=  1,		// 성공 
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
		items.m_classification = classification;  // 서버 종류 식별값
		items.m_startupNo = startupNo;  // 객체 생성 번호
	}
} ServerID;

//----------------------------------------------------
// 유저의 고유한 키 생성 
typedef union tagUID
{
	struct tagUDI
	{
		U32		nTime;		// 생성 시간 
		U16		nMap;		// 유저의 맵 넘버 
		U8		nSession;	// 세션 서버 넘버 
		U8		nSG;		// 서버 군 넘버 
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