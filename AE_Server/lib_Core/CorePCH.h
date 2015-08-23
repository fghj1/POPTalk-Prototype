// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

// WINNT 버젼 이상의 함수를 사용하기 위해 선언 
#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0501
#endif	


#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
// Windows 헤더 파일:
#include <windows.h>
#include <wchar.h>
#include <Pdh.h>

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
#include "include_Core.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#endif
#include <stdlib.h>
#include <crtdbg.h>


