#ifndef __STRINGINDEX_H
#define __STRINGINDEX_H

#include "AEType.h"

// String 자체를 인덱스로 사용할 때, 각각의 char을 하나씩 비교해야 하므로 부하가 큼
// 이 클래스는 문자열 비교를 U64형을 사용하여 부하를 줄임
// 단, '\0' 이후의 부분까지 비교하므로 완전히 Clear한 후 문자열을 복사해야 함

#define INDEXBINARYSIZE				4		// 속도를 생각해서 루프 사용 안합. 이부분이 바뀌면 si_Binary부분 모두 수정해야 함
#define INDEXSTRINGSIZE		(INDEXBINARYSIZE * sizeof(U64) / sizeof(TCHAR))

union StringIndex
{
private :
	U64   si_Binary[INDEXBINARYSIZE];
	TCHAR si_String[INDEXSTRINGSIZE];
public :
	inline StringIndex(void)
	{
		si_Clear();
	}
	inline StringIndex(const TCHAR *index)
	{
		si_Clear();
		_tcsncpy_s(si_String, index, INDEXSTRINGSIZE - 1);
	}
	inline ~StringIndex(void)
	{
	}
	inline void si_Clear(void)
	{
		si_Binary[0] = si_Binary[1] = si_Binary[2] = si_Binary[3] = 0;
	}
	inline bool operator ==(const StringIndex &si) const
	{
		return si_Binary[0] == si_Binary[0] &&
				si_Binary[1] == si_Binary[1] &&
				 si_Binary[2] == si_Binary[2] &&
				  si_Binary[3] == si_Binary[3];
	}
	inline bool operator !=(const StringIndex &si) const
	{
		return !(*this == si);
	}
	inline bool operator <(const StringIndex &si) const
	{
		if(si_Binary[0] < si.si_Binary[0])
			return true;
		else if(si_Binary[0] > si.si_Binary[0])
			return false;
		if(si_Binary[1] < si.si_Binary[1])
			return true;
		else if(si_Binary[1] > si.si_Binary[1])
			return false;
		if(si_Binary[2] < si.si_Binary[2])
			return true;
		else if(si_Binary[2] > si.si_Binary[2])
			return false;
		return si_Binary[3] < si.si_Binary[3];
	}
};

#endif __STRINGINDEX_H