// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

// �Ʒ� ������ �÷����� �켱�ϴ� �÷����� ������� �ϴ� ��� ���� ���Ǹ� �����Ͻʽÿ�.
// �ٸ� �÷����� ���Ǵ� �ش� ���� �ֽ� ������ MSDN�� �����Ͻʽÿ�.
#ifndef WINVER				// Windows XP �̻󿡼��� ����� ����� �� �ֽ��ϴ�.
#define WINVER 0x0501		// �ٸ� ������ Windows�� �µ��� ������ ������ ������ �ֽʽÿ�.
#endif

#ifndef _WIN32_WINNT		// Windows XP �̻󿡼��� ����� ����� �� �ֽ��ϴ�.                   
#define _WIN32_WINNT 0x0501	// �ٸ� ������ Windows�� �µ��� ������ ������ ������ �ֽʽÿ�.
#endif						

#ifndef _WIN32_WINDOWS		// Windows 98 �̻󿡼��� ����� ����� �� �ֽ��ϴ�.
#define _WIN32_WINDOWS 0x0410 // Windows Me �̻� �µ��� ������ ������ ������ �ֽʽÿ�.
#endif

#ifndef _WIN32_IE			// IE 6.0 �̻󿡼��� ����� ����� �� �ֽ��ϴ�.
#define _WIN32_IE 0x0600	// �ٸ� ������ IE�� �µ��� ������ ������ ������ �ֽʽÿ�.
#endif

#define WIN32_LEAN_AND_MEAN		// ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
// Windows ��� ����:

#include <objbase.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>

#if defined(UNICODE) | defined(_UNICODE)
#include <wchar.h> 
#endif 

#include <vector>
#include <queue>
#include <list>
#include <map>
#include <string>
#include <utility>


#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <windows.h>
#include <atlcoll.h>


using namespace std;


// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.

#include "../lib_Common/CommonPCH.h"
#include "../lib_Core/CorePCH.h"
#include "./include_Network.h"

using namespace AE_COMMON;

#pragma comment( lib, "Ws2_32.lib" )
#pragma comment( lib, "Mswsock.lib" )
