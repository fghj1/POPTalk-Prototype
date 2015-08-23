//-------------------------------------------------------------------------
//								A4StrCon.h
//-------------------------------------------------------------------------

#ifndef __A4STRCON_H__
#define __A4STRCON_H__


#include <windows.h>

namespace AE_COMMON
{
	template <class T>
	inline int TByteSize(const T *s)
	{
		int i;
		for (i = 0; s[i] != '\0'; i++);
#ifdef _UNICODE
		return (i<<1);
#else
		return i;
#endif
	}

	inline char* UIntToStringSpec(U32 value, char *s)
	{
		char temp[16];
		int pos = 0;
		do 
		{
			temp[pos++] = (char)('0' + value % 10);
			value /= 10;
		}
		while (value != 0);
		//int i;

		do
		*s++ = temp[--pos];
		while (pos > 0);
		*s = '\0';
		return s;
	}

	inline int ToWideString( WCHAR* &pwStr, const char* pStr, int len, BOOL IsEnd)
	{
		//ASSERT_POINTER(pStr, char);
		ASSERT(len >= 0 || len == -1);
		int nWideLen = MultiByteToWideChar(CP_ACP, 0, pStr, len, NULL, 0);
		if (len == -1)
		{
			--nWideLen;
		}
		if (nWideLen == 0)
		{
			return 0;
		}
		if (IsEnd)
		{
			pwStr = new WCHAR[(nWideLen+1)*sizeof(WCHAR)];
			ZeroMemory(pwStr, (nWideLen+1)*sizeof(WCHAR));
		}
		else
		{
			pwStr = new WCHAR[nWideLen*sizeof(WCHAR)];
			ZeroMemory(pwStr, nWideLen*sizeof(WCHAR));
		}
		MultiByteToWideChar(CP_ACP, 0, pStr, len, pwStr, nWideLen);
		return nWideLen;
	}

	inline int ToMultiBytes( char* &pStr, const WCHAR* pwStr, int len, BOOL IsEnd)
	{
		//ASSERT_POINTER(pwStr, WCHAR) ;
		ASSERT( len >= 0 || len == -1 ) ;
		int nChars = WideCharToMultiByte(CP_ACP, 0, pwStr, len, NULL, 0, NULL, NULL);
		if (len == -1)
		{
			--nChars;
		}
		if (nChars == 0)
		{
			return 0;
		}
		if(IsEnd)
		{
			pStr = new char[nChars+1];
			ZeroMemory(pStr, nChars+1);
		}
		else
		{
			pStr = new char[nChars];
			ZeroMemory(pStr, nChars);
		}
		WideCharToMultiByte(CP_ACP, 0, pwStr, len, pStr, nChars, NULL, NULL);
		return nChars;
	}
}
#endif
