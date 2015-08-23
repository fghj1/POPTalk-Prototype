//------------------------------------------------------------------------------
//						World_Enum.h	
//------------------------------------------------------------------------------


#ifndef __WORLD_ENUM__H__
#define __WORLD_ENUM__H__

//---------------------------------------------------------
// 메세지를 어디로 보낼지 지정하는 곳 
typedef enum tagEventType
{	
	TYPE_ACTOR_HANDLER,	
	TYPE_SKILL_HANDLER,	
	TYPE_ITEM_HANDLER,	
	TYPE_ENTITY_ATTRIBUTE,
	TYPE_CREATURE_STATE = 20,	   // NPC , 몬스터 상태 관련 
	TYPE_CREATURE_HANDLER,	
	TYPE_COMMON_HANDLER,
} eEventType;


typedef enum tagEventMSG
{
	MSG_Add_Object = 0,
	MSG_Remove_Object,
	MSG_Revive_Object,			// 오브젝트 다시 살어남 
	MSG_Stat_Addition_End,		// 부가효과 끝
	MSG_Recovery_HP,	
	MSG_Recovery_SP,	
	MSG_Recovery_GP,		
	MSG_Battle,	
	MSG_Casting,	
	MSG_Channeling_Repeat,	
	MSG_Channeling_Fire,
	MSG_Fire,
	MSG_CoolTime,	
	MSG_BasicCoolTime,	
	MSG_Stat_Death,	
	MSG_Release_Projectile,
	MSG_NormalAttackCoolTime,	// 평타 자동 공격 쿨타임
	MSG_Remove_Dead,			// 시체 제거

} eEventMSG;


typedef enum tagSkillActionType
{	
	ACTION_NONE = 0,	
	ACTION_SKILL,	
	ACTION_AUTO,	
} eSkillActionType;


#endif 
