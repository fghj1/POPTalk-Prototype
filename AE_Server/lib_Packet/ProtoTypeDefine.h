#pragma once

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
typedef unsigned __int64 U64;
typedef unsigned int serverID_t;
typedef unsigned __int64 objectHandle_t;
typedef unsigned int abstate_t;
typedef unsigned short abstakey_t;
typedef double aetime_t;
typedef float aetimespan_t;
typedef unsigned int hitpoint_t;
typedef unsigned int manapoint_t;
typedef float abstavalue_t;
typedef float distance_t;
typedef float aggro_t;
typedef int resource_t;
typedef double resourceProduction_t;
typedef unsigned char conquerFlag_t;
typedef int userNo_t;
typedef int lordNo_t;
typedef __int64 charNo_t;
typedef int villageNo_t;
typedef __int64 schedule_t;
typedef __int64 queueKey_t;
typedef unsigned __int64 itemKey_t;
typedef unsigned int itemNo_t;
typedef unsigned int skillNo_t;
typedef unsigned char level_t;
typedef unsigned char grade_t;
typedef unsigned int slotNo_t;
typedef int objectNo_t;
typedef short quantity_t;
typedef int mapID_t;
typedef int chNo_t;
typedef int chType_t;
typedef unsigned int zoneHandle_t;
typedef int sectorIndex_t;
typedef int questNo_t;
typedef int missionNo_t;
typedef unsigned int class_t;
typedef unsigned int stance_t;
typedef int customize_t;
typedef unsigned short optionNo_t;
typedef __int64 money_t;
typedef int honor_t;
typedef unsigned int cash_t;
typedef __int64 mailNo_t;
typedef __int64 reportNo_t;
typedef float scheduleSecond_t;
typedef int population_t;
typedef unsigned char buildingNo_t;
typedef unsigned char soldierNo_t;
typedef short villageScore_t;
typedef unsigned char loyalty_t;
typedef unsigned char morality_t;
typedef __int64 friendNo_t;
typedef short villageCoord_t;

enum MessageBoxBtn
{
	MB_NONE = 0,
	MB_NO = 1,
	MB_YES = 2,
	MB_CANCEL = 3,
	MB_CONFIRM = 4,
	MB_REVIVE = 5,
	MB_REVIVEBYITEM = 6,
};

enum StateObjectType
{
	SOT_NONE = 0,
	SOT_PEACE = 1,
	SOT_COMBAT = 2,
	SOT_TARGETING = 3,
	SOT_DEAD = 4,
	SOT_HIDE = 5,
	SOT_PROVOKE = 6,
	SOT_SILENCE = 7,
	SOT_STUN = 8,
	SOT_FEAR = 9,
	SOT_SHACKLE = 10,
	SOT_INVINCIBLE = 11,
	SOT_SLEEP = 12,
	SOT_RIDE = 13,
	SOT_LOOT = 14,
	SOT_HEAVY = 15,
	SOT_CHARGING = 16,
	SOT_ACTIONOBJECTOPEN = 17,
	SOT_NORMALLOGOUT = 18,
};

enum StateActType
{
	SAT_NONE = 0,
	SAT_MOVE_BY_ORDER = 1,
	SAT_TARGETING_BY_ORDER = 2,
	SAT_COMBAT_SKILL_BY_ORDER = 3,
	SAT_PIECE_SKILL_BY_ORDER = 4,
	SAT_ALWAYS_SKILL_BY_ORDER = 5,
	SAT_NORMALATTACK_BY_ORDER = 6,
	SAT_MOVE_BY_SKILL = 7,
	SAT_NORMALATTACK_BY_SYSTEM = 8,
	SAT_MOVE_BY_SYSTEM = 9,
	SAT_CAN_BE_SKILL_TARGET = 10,
	SAT_BE_DAMAGED = 11,
	SAT_RECOVER_IN_PEACE = 12,
	SAT_ITEM_EQUIP = 13,
	SAT_WEAPON_EQUIP = 14,
	SAT_LOOT = 15,
	SAT_ACTIONOBJECTOPEN = 16,
	SAT_MOVERRIDE = 17,
	SAT_RETURNSKILL = 18,
	SAT_NORMALLOGOUT = 19,
	SAT_DUPLICATEDLOGIN = 20,
	SAT_PORTALMOVE = 21,
	SAT_REVIVEBYITEM = 22,
};

enum PlayTimeLogType
{
	PT_LogIn = 0,
	PT_LogOut = 1,
	PT_LevelUp = 2,
};

enum LoginCondition
{
	LC_LOGIN_WAITING = 0,
	LC_LOGIN_WAITING_FOR_SESSIONREPLACEMENT = 1,
	LC_LOGIN = 2,
	LC_CHANGE_SERVER = 3,
	LC_LOGOUT_WAITING = 4,
	LC_COMPULSORY_LOGOUT_WAITING = 5,
	LC_COMPULSORY_LOGOUT = 6,
};

enum TradeType
{
	Trade_Buy = 0,
	Trade_Sell = 1,
};

enum ExpGetType
{
	EGT_MonsterKill = 0,
	EGT_Quest = 1,
	EGT_System = 2,
	EGT_MonsterRaid = 3,
};

enum ItemTradeType
{
	ItemTrade_Move = 0,
	ItemTrade_SplitMerge = 1,
	ItemTrade_SplitAdd = 2,
};

enum ItemOptionType
{
	ItemOption_RandomOption = 0,
	ItemOption_Durability = 1,
	ItemOption_StrengthenLevel = 2,
	ItemOption_StrengthenFailCount = 3,
};

enum ItemStorageType
{
	Storage_None = 0,
	Storage_Equipment = 1,
	Storage_InvenEquip = 2,
	Storage_InvenAcc = 3,
	Storage_InvenUsable = 4,
	Storage_InvenQuest = 5,
	Storage_InvenCash = 6,
	Storage_Warehouse = 7,
};

enum BattleFieldTeamType
{
	BFT_NEUTRAL = 0,
	BFT_RED = 1,
	BFT_BLUE = 2,
};

enum BattleFieldObjectType
{
	BFOT_PLAYER = 0,
	BFOT_NORMAL_NPC = 1,
	BFOT_WALL = 2,
	BFOT_TOWER = 3,
	BFOT_NEXUS = 4,
	BFOT_MAGIC_CRYSTAL = 5,
	BFOT_MAGIC_PIECE = 6,
};

enum BattleFieldObjectStatus
{
	BFO_ALIVE = 0,
	BFO_DESTROYED = 1,
};

enum ChannelType
{
	CT_NULL = 0,
	CT_Peace = 1,
	CT_Battle = 2,
	CT_BattleField = 3,
};

enum AIRegenType
{
	AI_RegenRepeat = 0,
	AI_RegenOnce = 1,
};

enum BattleFieldVoteInfo
{
	BFVR_Decide = 0,
	BFVR_Voteing = 1,
	BFVR_Reject = 2,
};

enum BattleFieldVote
{
	BFV_Assent = 0,
	BFV_Dissent = 1,
};

enum BattleFieldVotePossibleState
{
	BFVPS_Possible = 0,
	BFVPS_Impossible = 1,
};

enum FoeObjectChangeReason
{
	FoeAdd = 0,
	FoeRemove = 1,
	FoeReset = 2,
};

enum StrengthenResult
{
	StrengthenNone = 0,
	StrengthenSucc = 1,
	StrengthenDouble = 2,
	StrengthenKeep = 3,
	StrengthenBroken = 4,
};

enum LOGOUT_REASON
{
	LR_None = 0,
	LR_NormalLogout = 1,
	LR_BattleFieldEntry = 2,
	LR_ChangeServer = 3,
	LR_DuplicatedLogin = 4,
};

enum MoneyExchangeDirection
{
	Deposit = 0,
	Withdraw = 1,
};

enum LoginType
{
	LT_Lobby = 1,
	LT_Game = 2,
	LT_Inventory = 3,
	LT_Gamble = 4,
	LT_MonsterCity = 5,
	LT_MonsterResource = 6,
	LT_Scout = 7,
	LT_Oasis = 8,
	LT_Test = 100,
};

enum SpeechEventType
{
	SPEECH_EVENT_DEAD = 0,
	SPEECH_EVENT_IDLE = 1,
	SPEECH_EVENT_MOVE = 2,
	SPEECH_EVENT_BATTLE = 3,
};

enum MoneyType
{
	Money_Gold = 1,
	Money_Gem = 2,
	Money_FreeGem = 3,
};

