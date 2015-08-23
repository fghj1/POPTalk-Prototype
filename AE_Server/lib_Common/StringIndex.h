#ifndef __STRINGINDEX_H
#define __STRINGINDEX_H

#include "AEType.h"

// String ��ü�� �ε����� ����� ��, ������ char�� �ϳ��� ���ؾ� �ϹǷ� ���ϰ� ŭ
// �� Ŭ������ ���ڿ� �񱳸� U64���� ����Ͽ� ���ϸ� ����
// ��, '\0' ������ �κб��� ���ϹǷ� ������ Clear�� �� ���ڿ��� �����ؾ� ��

#define INDEXBINARYSIZE				4		// �ӵ��� �����ؼ� ���� ��� ����. �̺κ��� �ٲ�� si_Binary�κ� ��� �����ؾ� ��
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