

#ifndef __IRR_STRING_H_INCLUDED__
#define __IRR_STRING_H_INCLUDED__


#include <stdio.h>
#include <String.h>
#include <stdlib.h>
#include <windows.h>

#include "./AEType.h"
#include "./TAllocator.h"
#include "./TAutoPtr.h"
#include "./vformat.h"

using namespace frmt;


template <typename T, typename TAlloc = TAllocator<T> >
class String
{
public:

	//! Default constructor
	String() : array(0), allocated(1), used(1)
	{
		array = allocator.allocate(1); // new T[1];
		array[0] = 0x0;
	}

	//! Constructor
	String(const String<T>& other) : array(0), allocated(0), used(0)
	{
		*this = other;
	}

	//! Constructs a String from a float
	String(const double number) : array(0), allocated(0), used(0)
	{
		T tmpbuf[255];
		_sntprintf(tmpbuf, 255, _T( "%0.6f" ), number);
		*this = tmpbuf;
	}

	//! Constructs a String from an int
	String(int number)	: array(0), allocated(0), used(0)
	{
		// store if negative and make positive

		bool negative = false;
		if (number < 0)
		{
			number *= -1;
			negative = true;
		}

		// temporary buffer for 16 numbers

		char tmpbuf[16];
		tmpbuf[15] = 0;
		U32 idx = 15;

		// special case '0'

		if (!number)
		{
			tmpbuf[14] = '0';
			*this = &tmpbuf[14];
			return;
		}

		// add numbers

		while(number && idx)
		{
			--idx;
			tmpbuf[idx] = (char)('0' + (number % 10));
			number /= 10;
		}

		// add sign

		if (negative)
		{
			--idx;
			tmpbuf[idx] = '-';
		}

		*this = &tmpbuf[idx];
	}



	//! Constructs a String from an unsigned int
	String(unsigned int number)	: array(0), allocated(0), used(0)
	{
		// temporary buffer for 16 numbers

		char tmpbuf[16];
		tmpbuf[15] = 0;
		U32 idx = 15;

		// special case '0'

		if (!number)
		{
			tmpbuf[14] = '0';
			*this = &tmpbuf[14];
			return;
		}

		// add numbers

		while(number && idx)
		{
			--idx;
			tmpbuf[idx] = (char)('0' + (number % 10));
			number /= 10;
		}

		*this = &tmpbuf[idx];
	}



	//! Constructor for copying a String from a pointer with a given length
	template <class B>
	String(const B* const c, U32 length) : array(0), allocated(0), used(0)
	{
		if (!c)
		{
			// correctly init the String to an empty one
			*this="";
			return;
		}

		allocated = used = length+1;
		array = allocator.allocate(used); // new T[used];

		for (U32 l = 0; l<length; ++l)
			array[l] = (T)c[l];

		array[length] = 0;
	}



	//! Constructor for unicode and ascii strings
	template <class B>
	String(const B* const c) : array(0), allocated(0), used(0)
	{
		*this = c;
	}

	~String()
	{
		allocator.deallocate(array); // delete [] array;
	}


	operator const T*() const 
	{ 
		return array; 
	} 
	//! Assignment operator
	String<T>& operator=(const String<T>& other)
	{
		if (this == &other)
			return *this;

		allocator.deallocate(array); // delete [] array;
		allocated = used = other.size()+1;
		array = allocator.allocate(used); //new T[used];

		const T* p = other.c_str();
		for (U32 i=0; i<used; ++i, ++p)
			array[i] = *p;

		return *this;
	}



	//! Assignment operator for strings, ascii and unicode
	template <class B>
	String<T>& operator=(const B* const c)
	{
		if (!c)
		{
			if (!array)
			{
				array = allocator.allocate(1); //new T[1];
				allocated = 1;
			}
			used = 1;
			array[0] = 0x0;
			return *this;
		}

		if ((void*)c == (void*)array)
			return *this;

		U32 len = 0;
		const B* p = c;
		while(*p)
		{
			++len;
			++p;
		}

		// we'll take the old String for a while, because the new
		// String could be a part of the current String.
		T* oldArray = array;

		++len;
		allocated = used = len;
		array = allocator.allocate(used); //new T[used];

		for (U32 l = 0; l<len; ++l)
			array[l] = (T)c[l];

		allocator.deallocate(oldArray); // delete [] oldArray;
		return *this;
	}

	//! Add operator for other strings
	String<T> operator+(const String<T>& other) const
	{
		String<T> str(*this);
		str.append(other);

		return str;
	}

	//! Add operator for strings, ascii and unicode
	template <class B>
	String<T> operator+(const B* const c) const
	{
		String<T> str(*this);
		str.append(c);

		return str;
	}


	//! Direct access operator
	T& operator [](const U32 index)
	{
		ASSERT(index<=used); // bad index
		return array[index];
	}


	//! Direct access operator
	const T& operator [](const U32 index) const
	{
		ASSERT(index<=used); // bad index
		return array[index];
	}


	//! Comparison operator
	bool operator ==(const T* const str) const
	{
		if (!str)
			return false;

		U32 i;
		for(i=0; array[i] && str[i]; ++i)
			if (array[i] != str[i])
				return false;

		return !array[i] && !str[i];
	}



	//! Comparison operator
	bool operator ==(const String<T>& other) const
	{
		for(U32 i=0; array[i] && other.array[i]; ++i)
			if (array[i] != other.array[i])
				return false;

		return used == other.used;
	}


	//! Is smaller operator
	bool operator <(const String<T>& other) const
	{
		for(U32 i=0; array[i] && other.array[i]; ++i)
		{
			S32 diff = array[i] - other.array[i];
			if ( diff )
				return diff < 0;
		}

		return used < other.used;
	}

	//! Equals not operator
	bool operator !=(const T* const str) const
	{
		return !(*this == str);
	}

	//! Equals not operator
	bool operator !=(const String<T>& other) const
	{
		return !(*this == other);
	}

	bool	IsEmpty() const { return (used <= 1); }

	void Empty()
	{
		used = 1;
		array[0] = 0x00;
	}

	//! Returns length of String
	// \return Returns length of the String in characters. 
	U32 size() const
	{
		return used-1;
	}

	U32 bytesize() const
	{
#ifdef _UNICODE
		return ((used-1)<<1);
#else
		return (used-1);
#endif

	}

	//! Returns character String
	// \return Returns pointer to C-style zero terminated String. 
	T* c_str() const
	{
		return array;
	}



	//! Makes the String lower case.
	void make_lower()
	{
		for (U32 i=0; i<used; ++i)
			array[i] = ansi_lower ( array[i] );
	}



	//! Makes the String upper case.
	void make_upper()
	{
		const T a = (T)'a';
		const T z = (T)'z';
		const T diff = (T)'A' - a;

		for (U32 i=0; i<used; ++i)
		{
			if (array[i]>=a && array[i]<=z)
				array[i] += diff;
		}
	}

	bool equals_ignore_case(const String<T>& other) const
	{
		for(U32 i=0; array[i] && other[i]; ++i)
			if (ansi_lower(array[i]) != ansi_lower(other[i]))
				return false;

		return used == other.used;
	}


	bool lower_ignore_case(const String<T>& other) const
	{
		for(U32 i=0; array[i] && other.array[i]; ++i)
		{
			S32 diff = (S32) ansi_lower ( array[i] ) - (S32) ansi_lower ( other.array[i] );
			if ( diff )
				return diff < 0;
		}

		return used < other.used;
	}



	//! compares the first n characters of the strings
	bool equalsn(const String<T>& other, U32 n) const
	{
		U32 i;
		for(i=0; array[i] && other[i] && i < n; ++i)
			if (array[i] != other[i])
				return false;

		// if one (or both) of the strings was smaller then they
		// are only equal if they have the same length
		return (i == n) || (used == other.used);
	}


	//! compares the first n characters of the strings
	bool equalsn(const T* const str, U32 n) const
	{
		if (!str)
			return false;
		U32 i;
		for(i=0; array[i] && str[i] && i < n; ++i)
			if (array[i] != str[i])
				return false;

		// if one (or both) of the strings was smaller then they
		// are only equal if they have the same length
		return (i == n) || (array[i] == 0 && str[i] == 0);
	}


	// Appends a character to this String
	// \param character: Character to append. 
	void append(T character)
	{
		if (used + 1 > allocated)
			reallocate(used + 1);

		++used;

		array[used-2] = character;
		array[used-1] = 0;
	}

	// Appends a char String to this String
	// \param other: Char String to append. 
	void append(const T* const other)
	{
		if (!other)
			return;

		U32 len = 0;
		const T* p = other;
		while(*p)
		{
			++len;
			++p;
		}

		if (used + len > allocated)
			reallocate(used + len);

		--used;
		++len;

		for (U32 l=0; l<len; ++l)
			array[l+used] = *(other+l);

		used += len;
	}


	// Appends a String to this String
	// \param other: String to append.
	void append(const String<T>& other)
	{
		--used;
		U32 len = other.size()+1;

		if (used + len > allocated)
			reallocate(used + len);

		for (U32 l=0; l<len; ++l)
			array[used+l] = other[l];

		used += len;
	}


	// Appends a String of the length l to this String.
	// \param other: other String to append to this String.
	// param length: How much characters of the other String to add to this one. 
	void append(const String<T>& other, U32 length)
	{
		if (other.size() < length)
		{
			append(other);
			return;
		}

		if (used + length > allocated)
			reallocate(used + length);

		--used;

		for (U32 l=0; l<length; ++l)
			array[l+used] = other[l];
		used += length;

		// ensure proper termination
		array[used]=0;
		++used;
	}


	// Reserves some memory.
	// \param count: Amount of characters to reserve. 
	void reserve(U32 count)
	{
		if (count < allocated)
			return;

		reallocate(count);
	}


	//! finds first occurrence of character in String
	/** \param c: Character to search for.
	\return Returns position where the character has been found,
	or -1 if not found. */
	S32 findFirst(T c) const
	{
		for (U32 i=0; i<used; ++i)
			if (array[i] == c)
				return i;

		return -1;
	}

	//! finds first occurrence of a character of a list in String
	
	S32 findFirstChar(const T* const c, U32 count) const
	{
		if (!c)
			return -1;

		for (U32 i=0; i<used; ++i)
			for (U32 j=0; j<count; ++j)
				if (array[i] == c[j])
					return i;

		return -1;
	}


	//! Finds first position of a character not in a given list.
	template <class B>
	U32 findFirstCharNotInList(const B* const c, U32 count) const
	{
		for (U32 i=0; i<used-1; ++i)
		{
			U32 j;
			for (j=0; j<count; ++j)
				if (array[i] == c[j])
					break;

			if (j==count)
				return i;
		}

		return -1;
	}

	//! Finds last position of a character not in a given list.

	template <class B>
	S32 findLastCharNotInList(const B* const c, U32 count) const
	{
		for(S32 i=(S32)(used-2); i>=0; --i)
		{
			U32 j;
			for (j=0; j<count; ++j)
				if (array[i] == c[j])
					break;

			if (j==count)
				return i;
		}

		return -1;
	}

	//! finds next occurrence of character in String
	S32 findNext(T c, U32 startPos) const
	{
		for (U32 i=startPos; i<used; ++i)
			if (array[i] == c)
				return i;

		return -1;
	}


	//! finds last occurrence of character in String
	//! \param c: Character to search for.
	//! \param start: start to search reverse ( default = -1, on end )
	//! \return Returns position where the character has been found,
	//! or -1 if not found.
	S32 findLast(const T c, S32 start = -1) const
	{
		start = AE_COMMON::clamp ( start < 0 ? (S32)(used) - 1 : start, 0, (S32)(used) - 1 );
		for (S32 i=start; i>=0; --i)
			if (array[i] == c)
				return i;

		return -1;
	}

	//! finds last occurrence of a character of a list in String
	S32 findLastChar(const T* const c, U32 count) const
	{
		if (!c)
			return -1;

		for (S32 i=used-1; i>=0; --i)
			for (U32 j=0; j<count; ++j)
				if (array[i] == c[j])
					return i;

		return -1;
	}


	//! finds another String in this String
	//! \param str: Another String
	//! \return Returns positions where the String has been found,
	//! or -1 if not found.
	template <class B>
	S32 find(const B* const str) const
	{
		if (str && *str)
		{
			U32 len = 0;

			while (str[len])
				++len;

			if (len > used-1)
				return -1;

			for (U32 i=0; i<used-len; ++i)
			{
				U32 j=0;

				while(str[j] && array[i+j] == str[j])
					++j;

				if (!str[j])
					return i;
			}
		}

		return -1;
	}


	//! Returns a substring
	//! \param begin: Start of substring.
	//! \param length: Length of substring.
	String<T> subString(U32 begin, S32 length) const
	{
		if ((length+begin) > size())
			length = size()-begin;
		if (length <= 0)
			return String<T>("");

		String<T> o;
		o.reserve(length+1);

		for (S32 i=0; i<length; ++i)
			o.array[i] = array[i+begin];

		o.array[length] = 0;
		o.used = o.allocated;

		return o;
	}


	String<T>& operator += (T c)
	{
		append(c);
		return *this;
	}


	String<T>& operator += (const T* const c)
	{
		append(c);
		return *this;
	}


	String<T>& operator += (const String<T>& other)
	{
		append(other);
		return *this;
	}


	String<T>& operator += (const int i)
	{
		append(String<T>(i));
		return *this;
	}


	String<T>& operator += (const unsigned int i)
	{
		append(String<T>(i));
		return *this;
	}


	String<T>& operator += (const long i)
	{
		append(String<T>(i));
		return *this;
	}


	String<T>& operator += (const unsigned long& i)
	{
		append(String<T>(i));
		return *this;
	}


	String<T>& operator += (const double i)
	{
		append(String<T>(i));
		return *this;
	}


	String<T>& operator += (const float i)
	{
		append(String<T>(i));
		return *this;
	}


	//! replaces all characters of a special type with another one
	void replace(T toReplace, T replaceWith)
	{
		for (U32 i=0; i<used; ++i)
			if (array[i] == toReplace)
				array[i] = replaceWith;
	}

	String<T>& replace( U32 index, T character )
	{
		if( 0 <= index && index < ( used - 1 ) )
			array[index] = character;

		if( character == 0 )
		{
			U32 len = 0;
			const T* p = array;
			while( *p )
			{
				++len;
				++p;
			}
			used = len + 1;
		}

		return *this;
	}

	//! trims the String.
	// Removes whitespace from begin and end of the String.
	String<T>& trim()
	{
		const char whitespace[] = " \t\n\r";
		const U32 whitespacecount = 4;

		// find start and end of real String without whitespace
		S32 begin = findFirstCharNotInList(whitespace, whitespacecount);
		if (begin == -1)
			return (*this="");

		S32 end = findLastCharNotInList(whitespace, whitespacecount);

		return (*this = subString(begin, (end +1) - begin));
	}

	// trims right the String.
	String<T>& trimRight(const String<T>& other)
	{
		//S32 end = findLastCharNotInList(other.c_str(), other.size());
		S32 end = findLastChar(other.c_str(), other.size());	
		return (*this = subString(0, end));
	}

	// trims right the String.
	String<T>& trimLeft(const String<T>& other)
	{
		const char whitespace[] = " \t\n\r";
		const U32 whitespacecount = 4;
		// find start and end of real String without whitespace
		S32 begin = findFirstChar(other.c_str(), other.size());
		if (begin == -1)
			return (*this="");

		S32 end = findLastCharNotInList(whitespace, whitespacecount);

		return (*this = subString(begin+1, (end +1) - begin));
	}


	//! Erases a character from the String. May be slow, because all elements
	//! following after the erased element have to be copied.
	//! \param index: Index of element to be erased.
	void erase(U32 index)
	{
		ASSERT(index>=used); // access violation

		for (U32 i=index+1; i<used; ++i)
			array[i-1] = array[i];
		--used;
	}
	
	void format( const T* fmt, ...) 
	{
		//std::tstring sOut;
		std::basic_string<T, std::char_traits<T>, std::allocator<T> >	sOut;
		

		va_list vargs;
		va_start (vargs, fmt);
		vformat<T> (sOut, fmt, vargs);
		if(sOut.length()>0)
			(*this) = sOut.c_str();
		
		va_end (vargs);
	}
	/*
	void format(const T* fmt, ...) 
	{ 
		using std::tstring;
		using std::vector;

		if (NULL != fmt)
		{
			va_list marker = NULL; 
			va_start(marker, fmt); 
			size_t len = tVSCPRINTF(fmt, marker) + 1;

			vector<T> buffer(len, _T('\0'));

			int nWritten = tVSNPRINTF_S(&buffer[0], buffer.size(), len, fmt, marker);    

			if (nWritten > 0)
			{
				(*this) = &buffer[0];
			}

			// Reset variable arguments
			va_end(marker); 
		}
	} 
	*/
private:
	//! Returns a character converted to lower case
	inline T ansi_lower ( U32 x ) const
	{
		return x >= 'A' && x <= 'Z' ? (T) x + 0x20 : (T) x;
	}


	//! Reallocate the array, make it bigger or smaller
	void reallocate(U32 new_size)
	{
		T* old_array = array;

		array = allocator.allocate(new_size); //new T[new_size];
		allocated = new_size;

		U32 amount = used < new_size ? used : new_size;
		for (U32 i=0; i<amount; ++i)
			array[i] = old_array[i];

		if (allocated < used)
			used = allocated;

		allocator.deallocate(old_array); // delete [] old_array;
	}

	//--- member variables
	T*		array;
	U32		allocated;
	U32		used;
	TAlloc	allocator;
};


typedef String<char>		aCHAR;
typedef String<wchar_t>		wCHAR;



#ifdef _UNICODE
	typedef wCHAR sCHAR;
#else
	typedef aCHAR sCHAR;
#endif


//Converty MultiByte to WideChar	
inline wCHAR M2W(const aCHAR &srcString, UINT codePage = CP_ACP)
{
	wCHAR resultString;
	wchar_t* pszBuf = NULL;

	if(!srcString.IsEmpty())
	{
		int nBufSize = MultiByteToWideChar(codePage, 0, srcString, -1, (LPWSTR)pszBuf, 0);
		pszBuf = new wchar_t[nBufSize];
		memset (pszBuf, 0x00, sizeof(wchar_t)*nBufSize);
		MultiByteToWideChar(codePage, 0, srcString, -1, (LPWSTR)pszBuf, nBufSize);
		resultString = pszBuf;
		delete[] pszBuf;

	}
	return resultString;
}

//Converty WideChar to MultiByte
inline aCHAR W2M(const wCHAR &srcString, UINT codePage = CP_ACP)
{
	aCHAR resultString;
	char* pszBuf = NULL;
	if(!srcString.IsEmpty())
	{

		int nLenOfBuf = WideCharToMultiByte(codePage, 0, srcString, -1, NULL, 0, NULL, NULL);
		pszBuf = new char[nLenOfBuf];

		char defaultChar = '_';
		WideCharToMultiByte(codePage, 0, srcString, -1, (LPSTR)pszBuf, nLenOfBuf, &defaultChar, NULL);

		resultString = pszBuf;
		delete[] pszBuf;

	}
	return resultString;
}

#endif
