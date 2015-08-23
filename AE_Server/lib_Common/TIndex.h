//--------------------------------------------------------------------
//							TIndex.h	
//--------------------------------------------------------------------

#ifndef __TINDEX__H__
#define __TINDEX__H__

// Todo ªË¡¶.


#include "../lib_Common/CommonPCH.h"

#define		MAX_BIT_ARRAY_SIZE		3072


class TIndex 
{
public:
	TIndex::TIndex(void) : m_nCount(0)
	{		
		ClearAllNumber();	
	}

	TIndex::~TIndex(void)
	{
	}
private:
	DWORD m_nCount;
	BYTE m_ArrangedBit[MAX_BIT_ARRAY_SIZE];

	bool SetBit(int index)
	{
		int charIndex = index / 8;
		if( MAX_BIT_ARRAY_SIZE <= charIndex )
			return false;
		int bitIndex = index % 8;
		BYTE bitFlag = 0x80>>bitIndex;

		BITMASK_SET(m_ArrangedBit[charIndex], bitFlag);

		return true;
	}

	bool ResetBit(int index)
	{
		int charIndex = index / 8;
		if( MAX_BIT_ARRAY_SIZE <= charIndex )
			return false;
		int bitIndex = index % 8;
		UCHAR bitFlag = 0x80 >> bitIndex;

		BITMASK_REMOVE(m_ArrangedBit[charIndex], bitFlag);

		return true;
	}

	bool IsFullBit(BYTE  bit)
	{
		return (bit == 0xFF);
	}

	int GetSmallestBitIndex(BYTE bit)
	{
		BYTE bitFlag = 0x80;

		for(int i=0; i<8; i++)
		{
			if(BITMASK_CK( bit, (bitFlag >> i) )) continue;
			return i;
		}

		return -1;
	}

public:
	bool IsSet(int num)
	{
		int charIndex = num / 8;
		if( MAX_BIT_ARRAY_SIZE <= charIndex )
			return false;
		int bitIndex = num % 8;
		BYTE bitFlag = 0x80>>bitIndex;

		return BITMASK_CK(m_ArrangedBit[charIndex], bitFlag ) ? true : false;
	}

	int GetAvailNumber()
	{
		for(int i=0; i<MAX_BIT_ARRAY_SIZE; i++)
		{
			if(IsFullBit(m_ArrangedBit[i]))				continue;
			int availNum = GetSmallestBitIndex(m_ArrangedBit[i]) + (i*8);
			return availNum;
		}

		return -1;
	}

	bool AddNumber(int index)
	{
		if( SetBit(index) == false )
			return false;

		IncreaseCount();
		return true;
	}

	void TIndex::ClearAllNumber(void)
	{
		m_nCount = 0;
		memset(m_ArrangedBit, 0x00, sizeof(BYTE)*MAX_BIT_ARRAY_SIZE);
	}

	bool ReleaseNumber(int num)
	{
		if( ResetBit(num) == false )
			return false;

		DecreaseCount();
		return true;
	}

	inline int GetCount()	{	return m_nCount;	}
	inline VOID	IncreaseCount(VOID){InterlockedIncrement((LONG*) &m_nCount);}
	inline VOID	DecreaseCount(VOID){InterlockedDecrement((LONG*) &m_nCount);}

};






#endif 
