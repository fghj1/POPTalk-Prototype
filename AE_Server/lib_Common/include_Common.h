//------------------------------------------------------------------
//						Include_Common.h
//------------------------------------------------------------------


#ifndef __INCLUDE_COMMON__H__
#	define __INCLUDE_COMMON__H__

#	define TIXML_USE_STL

#	include <windows.h>
#	include "./AEDefine.h"
#	include "./AEType.h"
#	include "./AEUtil.h"
#	include "./AEMacro.h"

#	include "./vformat.h"
#	include "./AEStrCon.h"
#	include "./AEString.h"
#	include "./AEMathType.h"
#	include "./TCS.h"
#	include "./TSingleton.h"
#	include "./tinyxml.h"
#	include "./TIndex.h"

#	include "./AEServerDefine.h"

#	include "./AEThread.h"
#	include "./EnumBinder.h"
#	include "./RandomGenerator.h"
#	include "./WinService.h"

#	ifdef _DEBUG
#		if defined( _WIN64 ) || defined( WIN64 )
#			pragma comment( lib, "../ExternalLib/Debug/libtcmalloc_minimal64-debug.lib" )
#			pragma comment( lib, "../ExternalLib/Debug/tinyxmlSTL64_D.lib" )
#		else
#			pragma comment( lib, "../ExternalLib/Debug/libtcmalloc_minimal-debug.lib" )
#			pragma comment( lib, "../ExternalLib/Debug/tinyxmlSTL_D.lib" )
#		endif
#	else
#		if defined( _WIN64 ) || defined( WIN64 )
#			pragma comment( lib, "../ExternalLib/Release/libtcmalloc_minimal64.lib" )
#			pragma comment( lib, "../ExternalLib/Release/tinyxmlSTL64.lib" )
#		else
#			pragma comment( lib, "../ExternalLib/Release/libtcmalloc_minimal.lib" )
#			pragma comment( lib, "../ExternalLib/Release/tinyxmlSTL.lib" )
#		endif
#	endif
#endif
