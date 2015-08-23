//---------------------------------------------------------------------
//								A4Define.h
//---------------------------------------------------------------------

#ifndef __A4DEFINE__H__
#define __A4DEFINE__H__

//------------------------------------------
// Limits of exact-width integer types
//------------------------------------------
#define S8_MIN		(-128) 
#define S8_MAX		127
#define U8_MAX		0xff					// 255U 

#define S16_MIN		(-32768)
#define S16_MAX		32767
#define U16_MAX		0xffff					// 65535U 

#define S32_MIN		(-2147483647 - 1)
#define S32_MAX		2147483647
#define U32_MAX		0xffffffff				// 4294967295U 

#define S64_MIN		(-9223372036854775807LL - 1)
#define S64_MAX		9223372036854775807LL
#define U64_MAX		0xffffffffffffffffULL	// 18446744073709551615ULL 


const float P2UScale = 050.0f;
const float U2PScale = 0.02f;

const float MATH_PI = 3.1415927F;
const float MATH_EPSILON = 0.001f;

const float LootingRange = 5.0f;
const int	ObjectMoveUpdatePeriod = 250;		// ms

const int DurabilityPrecision = 100;
const float TradeDistance = 10.f;

const int TradeItemCount = 7;

const int StanceCount = 2;

const int InvalidPursuitCheckTime = 5000;		// ms
const unsigned __int64 charMoneyLimit = 9999999999;
const unsigned __int64 lordMoneyLimit = 9999999999;
const unsigned int invalidSlotNo = -1;

#define ABS( x )	( ( (x) >= 0 ) ? (x) : -(x) )
#define SQR( a ) ((a)*(a))

#define ANGLE( x )	( (x) * 0.017453292519943295f )
#define DEGREE( x ) ( (x) * 57.29577951308232f )

#define CONNECTIONPOOL

#endif 
