
// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // �� Windows ͷ���ų�����ʹ�õ�����
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ĳЩ CString ���캯��������ʽ��

// �ر� MFC ��ĳЩ�����������ɷ��ĺ��Եľ�����Ϣ������
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ


#include <afxdisp.h>        // MFC �Զ�����



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC �� Internet Explorer 4 �����ؼ���֧��
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC �� Windows �����ؼ���֧��
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // �������Ϳؼ����� MFC ֧��



//message enmu
#define TYPE_MOUSE_MOVE  0
#define TYPE_MOUSE_LEFT_DOWN  1
#define TYPE_MOUSE_LEFT_UP  2
#define TYPE_MOUSE_RIGHT_DOWN  3
#define TYPE_MOUSE_RIGHT_UP  4
#define TYPE_MOUSE_WHEEL  5
#define TYPE_KEY_DOWN  6
#define TYPE_KEY_UP  7
#define TYPE_MSG_HB 8//����
#define TYPE_MSG_FBL 9 //resolution
#define TYPE_MODEL_NOMAL  100
#define TYPE_MODEL_GAME  101
#define TYPE_BUINSESS_SERVERTYPE  1000//����ָ������
#define TYPE_BUINSESS_GETTOKEN  1001//�����ȡuserid��token
#define TYPE_BUINSESS_GETTOKEN_Rsps  2001//����userid��token
#define TYPE_BUINSESS_REBOOTMEDIA  2002//����RTSP����
#define TYPE_BUINSESS_RECONNECTMEDIA  2003//��������rtsp����
#define TYPE_BUINSESS_LOGOFF  2102   //[20180712]ע�� �ػ� ���������
#define TYPE_BUINSESS_SHUTDOWN  2103
#define TYPE_BUINSESS_REBOOT  2104
#define TYPE_TEST_LOOP_BETWEEN_SERVER_AND_CLIENT  2200 //test cycle time


//status enmu
#define  TYPE_STATUS_RUN_OK				0
#define  TYPE_STATUS_NET_OK				1
#define  TYPE_STATUS_NET_LOST			2
#define  TYPE_STATUS_PLAY_OK			3
#define  TYPE_STATUS_PLAY_CONNECTING	4 //��������
#define  TYPE_STATUS_PLAY_ERROR			5
#define  TYPE_STATUS_PLAY_CLOSE			6


//browser
#define WM_URL_CHANGED				(WM_APP+1)
#define WM_LOAD_CHANGED				(WM_URL_CHANGED+1)
#define WM_TITLE_CHANGED			(WM_LOAD_CHANGED+1)
#define WM_CREATE_NEW_PAGE			(WM_USER+1)
#define WM_START_REMOTE				(WM_USER+2)
#define WM_CLOSE_REMOTE				(WM_USER+3)

//������Ϣִ��
#define WM_KEYBOARD_MSG				(WM_USER+100)

#include "./rtspplayer/RtspClientLib.h"
#ifdef _DEBUG 
	#pragma comment(lib,".\\rtspplayerD\\RtspPlayerlib.lib")
#else 
	#pragma comment(lib,".\\rtspplayer\\RtspPlayerlib.lib")
#endif

//��־
#include "log4cplus\logger.h"
#include "log4cplus\fileappender.h"
#include "log4cplus\layout.h"
using namespace log4cplus;
#ifdef _DEBUG
	#pragma comment(lib,".\\rtspplayerD\\lib\\log4cplusSD.lib")
#else 
	#pragma comment(lib,".\\rtspplayer\\lib\\log4cplusS.lib")
#endif


#pragma comment(lib, "libcef.lib")

#ifdef _DEBUG
	#pragma comment(lib,".\\rtspplayerD\\libcef_dll_wrapper.lib")
#else 
	#pragma comment(lib,".\\rtspplayer\\libcef_dll_wrapper.lib")
#endif

using namespace std;
#pragma comment( lib, "ws2_32.lib" )

#include "WndsManger.h"
#include "Log.h"
string GetExePath();






#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


