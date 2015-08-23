//------------------------------------------------------------------------------
//						World_Enum.h	
//------------------------------------------------------------------------------


#ifndef __WORLD_ENUM__H__
#define __WORLD_ENUM__H__

//---------------------------------------------------------
// �޼����� ���� ������ �����ϴ� �� 
typedef enum tagEventType
{	
	TYPE_ACTOR_HANDLER,	
	TYPE_SKILL_HANDLER,	
	TYPE_ITEM_HANDLER,	
	TYPE_ENTITY_ATTRIBUTE,
	TYPE_CREATURE_STATE = 20,	   // NPC , ���� ���� ���� 
	TYPE_CREATURE_HANDLER,	
	TYPE_COMMON_HANDLER,
} eEventType;


typedef enum tagEventMSG
{
	MSG_Add_Object = 0,
	MSG_Remove_Object,
	MSG_Revive_Object,			// ������Ʈ �ٽ� �� 
	MSG_Stat_Addition_End,		// �ΰ�ȿ�� ��
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
	MSG_NormalAttackCoolTime,	// ��Ÿ �ڵ� ���� ��Ÿ��
	MSG_Remove_Dead,			// ��ü ����

} eEventMSG;


typedef enum tagSkillActionType
{	
	ACTION_NONE = 0,	
	ACTION_SKILL,	
	ACTION_AUTO,	
} eSkillActionType;


#endif 
