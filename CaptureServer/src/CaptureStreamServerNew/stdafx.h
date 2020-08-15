// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")


#include "MediaServerLib.h"
#ifdef _DEBUG
	#pragma comment(lib,"..\\..\\lib\\debug\\MediaServerLibD.lib")
#else 
	#pragma comment(lib,"..\\..\\lib\\release\\MediaServerLib.lib")
#endif


//��־
#include "log4cplus\logger.h"
#include "log4cplus\fileappender.h"
#include "log4cplus\layout.h"
using namespace log4cplus;
#ifdef _DEBUG
#pragma comment(lib,"..\\..\\lib\\log4app\\win32\\debug\\log4cplusSD.lib")
#else 
#pragma comment(lib,"..\\..\\lib\\log4app\\win32\\release\\log4cplusS.lib")
#endif

//capture
#include "WincaptureLib.h"
#ifdef _DEBUG
#pragma comment(lib,"..\\..\\lib\\debug\\WinCaptureAndEncodeLib.lib")
#else 
#pragma comment(lib,"..\\..\\lib\\release\\WinCaptureAndEncodeLib.lib")
#endif

//base64
#include "Base64CodecApi.h"
#ifdef _DEBUG
#pragma comment(lib,"..\\..\\lib\\base64\\debug\\Base64Codec.lib")
#else 
#pragma comment(lib,"..\\..\\lib\\base64\\release\\Base64Codec.lib")
#endif

//crptolib
//#ifdef _debug
//	#pragma comment(lib,"..\\..\\lib\\debug\\cryptolib.lib")
//#else 
//	#pragma comment(lib,"..\\..\\lib\\release\\cryptolib.lib")
//#endif

#include "Log.h"
#include "NetComdef.h"
string GetExePath();


