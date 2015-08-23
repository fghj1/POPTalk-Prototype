//----------------------------------------------------------------
//						ServerIndex.h
//----------------------------------------------------------------

#ifndef __SERVER_INDEX__H__
#define __SERVER_INDEX__H__

// 서버 타입 설정 
typedef enum tagServerIdx
{
	S_NONE = 0,
	S_LOGIN,
	S_SESSION,
	S_GAME,
	S_PLANET,
	S_NPC,
	S_COMMUNITY,
	S_DBAGENT,
	S_LOG,
	S_END,
} eServerIdx;

typedef enum tagRetValue
{
	E_Continue = 1,
	E_Abort    = 2,
	E_Blocked  = 3

} eRetValue;


#endif 