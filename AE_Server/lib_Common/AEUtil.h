//---------------------------------------------------------------------------
//								A4Util.h	
//---------------------------------------------------------------------------

#ifndef __A4UTIL__H__
#define __A4UTIL__H__


#include <map>
#include <time.h>
#include <math.h>
#include<vector>
#include <MMSystem.h>
#include "./AEType.h"
#include <OleAuto.h>
using namespace std;

namespace AE_COMMON
{


//반올림하기
inline int round(float f) { return f - 0.5 >(int)f ?(int)f + 1 :(int)f; }


inline int getPercentValue(int value, int percent)
{
	return(int)((float)value *(float)percent / 100.0);
}


inline U64 MakeSerial(S8 sessionID, S8 worldID, U16 nAdder )
{
	
	U64	Serial;
	DWORD dwTime = timeGetTime();
	if( nAdder > 30000 )
		nAdder = 0;
	Serial = worldID;					
	Serial = sessionID	| Serial << 8;
	Serial = nAdder		| Serial << 16;
	Serial = dwTime	| Serial << 32;
	return Serial;
}

#ifdef KW_FAST_MATH
	const F32 ROUNDING_ERROR_32 = 0.00005f;
	const F64 ROUNDING_ERROR_64 = 0.000005f;
#else
	const F32 ROUNDING_ERROR_32 = 0.000001f;
	const F64 ROUNDING_ERROR_64 = 0.00000001f;
#endif

	//! returns minimum of two values. Own implementation to get rid of the STL (VS6 problems)
	template <class T> 
	inline void	swap_( T& a, T& b )	
	{ 
		T temp; temp = a; a = b; b = temp; 
	}

	template<class T>
	inline const T& min_(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	//! returns minimum of three values.
	template<class T>
	inline const T& min_(const T& a, const T& b, const T& c)
	{
		return a < b ? min_(a, c) : min_(b, c);
	}

	//! returns maximum of two values. Own implementation to get rid of the STL (VS6 problems)
	template<class T>
	inline const T& max_(const T& a, const T& b)
	{
		return a < b ? b : a;
	}


	//! returns maximum of three values. Own implementation to get rid of the STL (VS6 problems)
	template<class T>
	inline const T& max_(const T& a, const T& b, const T& c)
	{
		return a < b ? max_(b, c) : max_(a, c);
	}

	//! returns abs of two values. Own implementation to get rid of STL (VS6 problems)
	template<class T>
	inline T abs_(const T& a)
	{
		return a < (T)0 ? -a : a;
	}

	//! returns linear interpolation of a and b with ratio t
	//! \return: a if t==0, b if t==1, and the linear interpolation else
	template<class T>
	inline T lerp(const T& a, const T& b, const F32 t)
	{
		return (T)(a*(1.f-t)) + (b*t);
	}

	//! clamps a value between low and high
	template <class T>
	inline const T clamp (const T& value, const T& low, const T& high)
	{
		return min_ (max_(value,low), high);
	}

	//! returns if a equals b, taking possible rounding errors into account
	inline bool equals(const F32 a, const F32 b, const F32 tolerance = ROUNDING_ERROR_32)
	{
		return (a + tolerance >= b) && (a - tolerance <= b);
	}

	//! returns if a equals b, taking possible rounding errors into account
	inline bool equals(const S32 a, const S32 b, const S32 tolerance = 0)
	{
		return (a + tolerance >= b) && (a - tolerance <= b);
	}

	//! returns if a equals b, taking possible rounding errors into account
	inline bool equals(const U32 a, const U32 b, const U32 tolerance = 0)
	{
		return (a + tolerance >= b) && (a - tolerance <= b);
	}

	//! returns if a equals zero, taking rounding errors into account
	inline bool iszero(const F32 a, const F32 tolerance = ROUNDING_ERROR_32)
	{
		return fabsf ( a ) <= tolerance;
	}

	//! returns if a equals zero, taking rounding errors into account
	inline bool iszero(const S32 a, const S32 tolerance = 0)
	{
		return ( a & 0x7ffffff ) <= tolerance;
	}

	//! returns if a equals zero, taking rounding errors into account
	inline bool iszero(const U32 a, const U32 tolerance = 0)
	{
		return a <= tolerance;
	}

	inline S32 S32_min ( S32 a, S32 b)
	{
		S32 mask = (a - b) >> 31;
		return (a & mask) | (b & ~mask);
	}

	inline S32 S32_max ( S32 a, S32 b)
	{
		S32 mask = (a - b) >> 31;
		return (b & mask) | (a & ~mask);
	}

	inline S32 s32_clamp (S32 value, S32 low, S32 high)
	{
		return S32_min (S32_max(value,low), high);
	}

	inline F32 F32_max3(const F32 a, const F32 b, const F32 c)
	{
		return a > b ? (a > c ? a : c) : (b > c ? b : c);
	}

	inline F32 F32_min3(const F32 a, const F32 b, const F32 c)
	{
		return a < b ? (a < c ? a : c) : (b < c ? b : c);
	}

	inline F32 fract ( F32 x )
	{
		return x - floorf ( x );
	}

	inline unsigned __int64 GetLocalTimestampFromVariantTime(double variantTime)
	{
		SYSTEMTIME systemTime;
		VariantTimeToSystemTime(variantTime, &systemTime);
		tm t;
		t.tm_year = systemTime.wYear-1900;
		t.tm_mon = systemTime.wMonth-1;
		t.tm_mday = systemTime.wDay;
		t.tm_hour = systemTime.wHour;
		t.tm_min = systemTime.wMinute;
		t.tm_sec = systemTime.wSecond;
		t.tm_isdst = -1;

		time_t localTime = _mkgmtime(&t);

		if (localTime == (time_t)(-1))
			localTime = 0;
		else
			localTime -= 32400;

		return localTime;
	}

	inline time_t GetUTCTimeFromVariantTime(double variantTime)
	{
		SYSTEMTIME systemTime;
		VariantTimeToSystemTime(variantTime, &systemTime);
		tm t;
		t.tm_year = systemTime.wYear-1900;
		t.tm_mon = systemTime.wMonth-1;
		t.tm_mday = systemTime.wDay;
		t.tm_hour = systemTime.wHour;
		t.tm_min = systemTime.wMinute;
		t.tm_sec = systemTime.wSecond;
		t.tm_isdst = -1;

		time_t utcTime = _mkgmtime(&t);

		return utcTime;
	}

} // end namespace AECOMMON



typedef struct tagTAUTO_MEM_RELEASE 
{
	void operator()(void *p) 
	{ 	
		if (p)		
			free(p);	
	}
} TAUTO_MEM_RELEASET;



template<class T>
struct DeleteStruct
{
	void operator()(std::pair<U32, T> p) const
	{
		SAFE_DELETE(p.second);	
	}
};


//-----------------------------------------------------------
//	recursive template permutation function
//	Note : func parameters must always be vector<T> !
//-----------------------------------------------------------
template <typename T, typename Func>
void vector_permutation(std::vector<T>& now, std::vector<T> next, Func func)
{
	int size=now.size();
	if(size>0)
	{
		for(int cnt=0; cnt<size;++cnt)
		{
			std::vector<T> vt;

			std::vector<T>::const_iterator it=now.begin();
			for(int cnt1=0;cnt1<size;++cnt1)
			{
				if(cnt1==cnt)
				{
					++it;
					continue;
				}
				else
					vt.push_back(*it);
				++it;  
			}

			std::vector<T>::const_iterator it1=now.begin();
			--it1;
			for(int cnt2=0;cnt2<=cnt;++cnt2)
			{
				++it1;
			}

			next.push_back(*it1);
			vector_permutation(vt,next,func);
			next.pop_back();        
		}

	}          
	else
	{
		func(next);
	}  

	//vector<char> ca;
	//vector<char> cnext;
	//vector_permutation(ca,cnext,display);
}

inline UINT GetDeltaTime(UINT pastTime, UINT currentTime)
{
	if (pastTime > currentTime)
	{
		return 0xffffffff -pastTime +currentTime;
	}
	else
	{
		return currentTime -pastTime;
	}
}

#endif 
