//--------------------------------------------------------------------------
//								A4Type.h
//--------------------------------------------------------------------------


#ifndef __A4TYPE__H__
#define __A4TYPE__H__

#include <tchar.h>
#include <assert.h>

#include "../lib_Packet/ProtoTypeDefine.h"


typedef signed char				S8;     ///< Compiler independent signed char (8bit integer).
typedef signed short			S16;    ///< Compiler independent signed 16-bit short integer.
typedef signed int				S32;    ///< Compiler independent signed 32-bit integer.
typedef unsigned int			U32;    ///< Compiler independent unsigned 32-bit integer.

typedef signed long				SL32;    ///< Compiler independent signed 32-bit long.
typedef unsigned long			UL32;    ///< Compiler independent unsigned 32-bit long.

typedef float					F32;    ///< Compiler independent 32-bit float.
typedef double					F64;    ///< Compiler independent 64-bit float.
typedef signed __int64			S64;	///< Compiler independent signed 64-bit integer.
typedef unsigned __int64		U64;	///< Compiler independent unsigned 64-bit integer.


//using Number = System.Int32;


// define how we export the functions
#ifdef _WIN32
#	define	WIN_A4_API	__stdcall
#else
#	define	WIN_A4_API
#endif


#ifndef USE_MFC
	#ifndef ASSERT
		#ifdef _DEBUG
	#define ASSERT assert
	#else
	#define ASSERT
		#endif // _DEBUG
	#endif // !ASSERT

#endif //!USE_MFC

#ifdef _DEBUG
#	ifndef ASSERT
#		define ASSERT(x) do { if (!(x)) InlDebugBreak(); } while (false)
#	endif // ASSERT
#	ifndef VERIFY
#		define VERIFY(x) ASSERT(x)
#	endif
#else // _DEBUG
#	ifndef ASSERT
#		define ASSERT(x)
#	endif // ASSERT
#	ifndef VERIFY
#		define VERIFY(x) (x)
#	endif
#endif // _DEBUG

#if _MSC_VER < 1400

#ifndef _tfopen_s
#define _tfopen_s( pFile, filename, mode ) \
	((*(pFile) = _tfopen( (filename), (mode) )) != NULL ? 0 : 1)
#endif

#ifndef _tcscpy_s
#define _tcscpy_s( strDestination, sizeInChars, strSource ) \
	_tcscpy( (strDestination), (strSource) )
#endif

#ifndef _tcscat_s
#define _tcscat_s( strDestination, sizeInChars, strSource ) \
	_tcscat( (strDestination), (strSource) )
#endif

#ifndef _taccess_s
#define _taccess_s( path, mode ) \
	_taccess( (path), (mode) )
#endif

#endif


#if defined(UNICODE) | defined(_UNICODE)
	//typedef LPCWSTR				LPCTSTR; 
	typedef wchar_t					TCHAR;
	//typedef wchar_t				TCHAR;
	typedef unsigned short			tUCHAR;

	#define tPRINTF			wprintf
	#define tSPRINTF		swprintf
	#define tSPRINTF_S		swprintf_s
	#define tVPRINTF		vwprintf
	#define tVSPRINTF		vswprintf
	#define tSNPRINTF		_snwprintf
	#define tSNPRINTF_S		_snwprintf_s
	

	#define tVSNPRINTF		_vsnwprintf
	#define tVSNPRINTF_S	_vsnwprintf_s

	#define tITOA			_itow
	#define tLTOA			_ltow
	#define tULTOA			_ultow
	#define tATOI			_wtoi
	#define tATOL			_wtol
	#define tATOF			_wtof
	#define tSTRTOL			wcstol
	#define tSTRTOUL		wcstoul
	#define tSTRTOD			wcstod
	#define tSTRLEN			wcslen
	#define tSTRCPY			wcscpy
	#define tSTRNCPY		wcsncpy
	#define tSTRCAT			wcscat
	#define tSTRNCAT		wcsncat
	#define tSTRCMP			wcscmp
	#define tSTRNCMP		wcsncmp
	#define tSTRICMP		_wcsicmp
	#define tSTRNICMP		_wcsnicmp
	#define tSTRCHR			wcschr
	#define tSTRRCHR		wcsrchr
	#define tSTRSTR			wcsstr
	#define tSTRPBRK		wcspbrk
	#define tSTRTOK			wcstok
	#define tSTRSET     	_wcsset
	#define tSTRNSET    	_wcsnset
	#define tSTRLWR     	_wcslwr
	#define tSTRUPR     	_wcsupr
	#define tSTRREV     	_wcsrev
	#define tSTRDUP     	_wcsdup
	#define tISALNUM    	iswalnum
	#define tISALPHA    	iswalpha
	#define tISDIGIT    	iswdigit
	#define tISGRAPH    	iswgraph
	#define tISLOWER    	iswlower
	#define tISPRINT    	iswprint
	#define tISPUNCT    	iswpunct
	#define tISSPACE    	iswspace
	#define tISUPPER    	iswupper
	#define tTOUPPER    	towupper
	#define tTOLOWER    	towlower

	

	#define tSTRCPY_S   	wcscpy_s
	#define tSTRNCPY_S  	wcsncpy_s
	#define tSTRCAT_S   	wcscat_s
	#define tSTRNCAT_S  	wcsncat_s
	#define tSNPRINTF_S 	_snwprintf_s
	#define tSPRINTF_S  	swprintf_s

	#define tGETENV     	_wgetenv
	#define tMKDIR			_wmkdir
	#define tACCESS	    	_waccess
	#define tCHMOD			_wchmod
	#define tSTAT			_wstat
	#define tFOPEN	    	_wfopen
	#define tFOPEN_S	    _wfopen_s
	#define tFPUTS	    	fputws
	       
	#define tFPRINTF		fwprintf
	#define tPERROR     	_wperror
	#define tVSPRINTF		vswprintf
	#define tVSCPRINTF		_vscwprintf
	#define tVSCPRINTF_L	_vscwprintf_l

	#define tVSPRINTF_S		 vswprintf_s
	#define tSPLITPATH  	_wsplitpath
	#define tSTRFTIME   	 wcsftime
	#define tUNLINK     	_wunlink

	#define tSTRFTIME		wcsftime
	

	// time func 
	#define tSTRDATE		_wstrdate
	#define tSTRDATE_S		_wstrdate_s

	#define tSTRTIME		_wstrtime 
	#define tSTRTIME_S		_wstrtime_s 


	



#else
	typedef LPCSTR			LPCTSTR;
	typedef char			TCHAR;
	typedef unsigned char	tUCHAR;

	#define tPRINTF			printf
	#define tSPRINTF		sprintf
	#define tSPRINTF_S		sprintf_s

	#define tVPRINTF		vprintf
	#define tVSPRINTF		vsprintf

	#define tSNPRINTF		_snprintf
	#define tSNPRINTF_S		_snprintf_s
	#define tVSNPRINTF		_vsnprintf
	#define tVSNPRINTF_S	_vsnprintf_s

	#define tITOA			itoa
	#define tLTOA			ltoa
	#define tULTOA			ultoa
	#define tATOI			atoi
	#define tATOL			atol
	#define tATOF			atof
	#define tSTRTOL			strtol
	#define tSTRTOUL		strtoul
	#define tSTRTOD			strtod
	#define tSTRLEN			strlen
	#define tSTRCPY			strcpy
	#define tSTRNCPY		strncpy
	#define tSTRCAT			strcat
	#define tSTRNCAT		strncat
	#define tSTRCMP			strcmp
	#define tSTRNCMP		strncmp
	#define tSTRICMP		stricmp
	#define tSTRNICMP		strnicmp
	#define tSTRCHR			strchr
	#define tSTRRCHR		strrchr
	#define tSTRSTR			strstr
	#define tSTRPBRK		strpbrk
	#define tSTRTOK			strtok
	#define tSTRSET			strset
	#define tSTRNSET		strnset
	#define tSTRLWR			strlwr
	#define tSTRUPR			strupr
	#define tSTRREV			strrev
	#define tSTRDUP			_mbsdup
	#define tISALNUM		isalnum
	#define tISALPHA		isalpha
	#define tISDIGIT		isdigit
	#define tISGRAPH		isgraph
	#define tISLOWER		islower
	#define tISPRINT		isprint
	#define tISPUNCT		ispunct
	#define tISSPACE		isspace
	#define tISUPPER		isupper
	#define tTOUPPER		toupper
	#define tTOLOWER		tolower

	#define tSTRCPY_S		strcpy_s
	#define tSTRNCPY_S		strncpy_s
	#define tSTRCAT_S		strcat_s
	#define tSTRNCAT_S		strncat_s
	#define tSNPRINTF_S		_snprintf_s
	#define tSPRINTF_S		sprintf_s

	#define tGETENV			getenv
	#define tMKDIR			_mkdir
	#define tACCESS			_access
	#define tCHMOD			_chmod
	#define tSTAT			_tstat
	#define tFOPEN			fopen
	#define tFOPEN_S	    fopen_s
	#define tFPUTS	    	fputs

	
	#define tFPRINTF		fprintf
	#define tPERROR			perror
	#define tVSPRINTF		vsprintf
	#define tVSCPRINTF		_vscprintf
	#define tVSCPRINTF_L	_vscprintf_l

	#define tSPLITPATH		_splitpath
	#define tSTRFTIME		strftime
	#define tUNLINK			_unlink
	#define tSTRFTIME		strftime
	#define tSTRDATE		_strdate
	#define tSTRDATE_S		_strdate_s

	#define tSTRTIME		_strtime
	#define tSTRTIME_S		_strtime_s




	//#ifdef 
	#define tVSPRINTF_S		vsprintf_s
	//#endif

#endif



#include <string>
namespace std
{
	#if defined UNICODE || defined _UNICODE

		typedef wstring         tstring;

		typedef wstringbuf      tstringbuf;
		typedef wstringstream   tstringstream;
		typedef wostringstream  tostringstream;
		typedef wistringstream  tistringstream;

		typedef wstreambuf      tstreambuf;

		typedef wistream        tistream;
		typedef wiostream       tiostream;

		typedef wostream        tostream;

		typedef wfilebuf        tfilebuf;
		typedef wfstream        tfstream;
		typedef wifstream       tifstream;
		typedef wofstream       tofstream;

		typedef wios            tios;

		#define tcerr            wcerr
		#define tcin             wcin
		#define tclog            wclog
		#define tcout            wcout

	#else // defined UNICODE || defined _UNICODE

		typedef string          tstring;

		typedef stringbuf       tstringbuf;
		typedef stringstream    tstringstream;
		typedef ostringstream   tostringstream;
		typedef istringstream   tistringstream;


		typedef streambuf       tstreambuf;

		typedef istream         tistream;
		typedef iostream        tiostream;

		typedef ostream         tostream;

		typedef filebuf         tfilebuf;
		typedef fstream         tfstream;
		typedef ifstream        tifstream;
		typedef ofstream        tofstream;

		typedef ios             tios;

		#define tcerr            cerr
		#define tcin             cin
		#define tclog            clog
		#define tcout            cout

	#endif // defined UNICODE || defined _UNICODE
} // namespace std


// TYPEDEF by Chamsol
// 패킷에 필요한 타입들은 "../lib_Packet/ProtoTypeDefine.h"으로 모두 옮길것
typedef S32			stat_t;
typedef float		ability_t;
typedef U16			count_t;
typedef float		distance_t;
typedef double      parameter_t;	// 파라메터(str, con, movespeed 등)

#define INVALID_OBJECT   ((objectHandle_t)0)
#define INVALID_ITEMNO   ((itemNo_t)0)
#define INVALID_VILLAGE	 ((villageNo_t)0)
#define INVALID_LORD	 ((lordNo_t)0)
#define INVALID_CHAR	 ((charNo_t)0)
#define INVALID_MAP		 ((mapID_t)0)
#define INVALID_ITEMKEY	 ((itemKey_t)0)

typedef skillNo_t         skillid_t;
#define INVALID_SKILL    ((skillNo_t)0)

typedef U32         projectileid_t;

#define INVALID_ABSTATE	 ((abstate_t)0)

#define INVALID_PARTY    (int)0

#define ArraySize(arr)		(sizeof(arr) / sizeof(*arr))

#endif
