///////////////////////////////////////////
// EventSelectServer.cpp�ļ�

#include "stdafx.h"
#include "./common/initsock.h"

#include <stdio.h>
#include <windows.h>

#include "EventSelectServer.h"
#include "MsgQueue.h"
#include "Util.h"

#include <process.h>
#include <string>
using std::string;

#define   MOUSE_RATE_BASE    15000
#define   MOUSE_RATE         4

DWORD WINAPI ServerThread(LPVOID lpParam);
// �߳��б�
PTHREAD_OBJ g_pThreadList;		// ָ���̶߳����б��ͷ
CRITICAL_SECTION g_cs;			// ͬ���Դ�ȫ�ֱ����ķ���


// ״̬��Ϣ
LONG g_nTatolConnections;		// �ܹ���������
LONG g_nCurrentConnections;		// ��ǰ��������

// ��ʼ��Winsock��
CInitSock theSock;
OnEventUserIOStatus g_IoStatusCallback = NULL;

////��ǰ�������ģʽ
//int g_nMouseModel = 0;
//
//int nWidth = 1280;
//int nHeight = 720;

// ����һ���׽��ֶ��󣬳�ʼ�����ĳ�Ա
PSOCKET_OBJ GetSocketObj(SOCKET s)
{
	PSOCKET_OBJ pSocket = (PSOCKET_OBJ)::GlobalAlloc(GPTR, sizeof(SOCKET_OBJ));
	if (pSocket != NULL)
	{
		pSocket->s = s;
		pSocket->event = ::WSACreateEvent();
		pSocket->bSendTag = false;
		pSocket->bBeginHeatbeat = false;
		pSocket->pHeartInfo = NULL;
	}
	return pSocket;
}

// �ͷ�һ���׽��ֶ���
void FreeSocketObj(PSOCKET_OBJ pSocket)
{
	::CloseHandle(pSocket->event);
	if (pSocket->s != INVALID_SOCKET)
	{
		::closesocket(pSocket->s);
	}
	if (pSocket->pHeartInfo != NULL)
	{
		delete pSocket->pHeartInfo;
		pSocket->pHeartInfo = NULL;
	}
	::GlobalFree(pSocket);
	pSocket = NULL;
}

// ����һ���̶߳��󣬳�ʼ�����ĳ�Ա����������ӵ��̶߳����б���
PTHREAD_OBJ GetThreadObj()
{
	PTHREAD_OBJ pThread = (PTHREAD_OBJ)::GlobalAlloc(GPTR, sizeof(THREAD_OBJ));
	if (pThread != NULL)
	{
		::InitializeCriticalSection(&pThread->cs);
		// ����һ���¼���������ָʾ���̵߳ľ��������Ҫ����
		pThread->events[0] = ::WSACreateEvent();

		// ����������̶߳�����ӵ��б���
		::EnterCriticalSection(&g_cs);
		pThread->pNext = g_pThreadList;
		g_pThreadList = pThread;
		::LeaveCriticalSection(&g_cs);
	}
	return pThread;
}

// �ͷ�һ���̶߳��󣬲��������̶߳����б����Ƴ�
void FreeThreadObj(PTHREAD_OBJ pThread)
{
	// ���̶߳����б��в���pThread��ָ�Ķ�������ҵ��ʹ����Ƴ�
	::EnterCriticalSection(&g_cs);
	PTHREAD_OBJ p = g_pThreadList;
	if (p == pThread)		// �ǵ�һ����
	{
		g_pThreadList = p->pNext;
	}
	else
	{
		while (p != NULL && p->pNext != pThread)
		{
			p = p->pNext;
		}
		if (p != NULL)
		{
			// ��ʱ��p��pThread��ǰһ��������p->pNext == pThread��
			p->pNext = pThread->pNext;
		}
	}
	::LeaveCriticalSection(&g_cs);

	// �ͷ���Դ
	::CloseHandle(pThread->events[0]);
	::DeleteCriticalSection(&pThread->cs);
	::GlobalFree(pThread);
}

// ���½����̶߳����events����
void RebuildArray(PTHREAD_OBJ pThread)
{
	::EnterCriticalSection(&pThread->cs);
	PSOCKET_OBJ pSocket = pThread->pSockHeader;
	int n = 1;	// �ӵ�1����ʼд����0������ָʾ��Ҫ�ؽ���
	while (pSocket != NULL)
	{
		pThread->events[n++] = pSocket->event;
		pSocket = pSocket->pNext;
	}
	::LeaveCriticalSection(&pThread->cs);
}

/////////////////////////////////////////////////////////////////////

// ��һ���̵߳��׽����б��в���һ���׽���
BOOL InsertSocketObj(PTHREAD_OBJ pThread, PSOCKET_OBJ pSocket)
{
	BOOL bRet = FALSE;
	::EnterCriticalSection(&pThread->cs);
	if (pThread->nSocketCount < WSA_MAXIMUM_WAIT_EVENTS - 1)
	{
		if (pThread->pSockHeader == NULL)
		{
			pThread->pSockHeader = pThread->pSockTail = pSocket;
		}
		else
		{
			pThread->pSockTail->pNext = pSocket;
			pThread->pSockTail = pSocket;
		}
		pThread->nSocketCount++;
		bRet = TRUE;
	}
	::LeaveCriticalSection(&pThread->cs);

	// ����ɹ���˵���ɹ������˿ͻ�����������
	if (bRet)
	{
		::InterlockedIncrement(&g_nTatolConnections);
		::InterlockedIncrement(&g_nCurrentConnections);
	}
	if (g_IoStatusCallback != NULL)
	{
		g_IoStatusCallback(0, NULL);
	}
	return bRet;
}

// ��һ���׽��ֶ����Ÿ����е��̴߳���
void AssignToFreeThread(PSOCKET_OBJ pSocket)
{
	pSocket->pNext = NULL;

	::EnterCriticalSection(&g_cs);
	PTHREAD_OBJ pThread = g_pThreadList;
	// ��ͼ���뵽�ִ��߳�
	while (pThread != NULL)
	{
		if (InsertSocketObj(pThread, pSocket))
			break;
		pThread = pThread->pNext;
	}

	// û�п����̣߳�Ϊ����׽��ִ����µ��߳�
	if (pThread == NULL)
	{
		pThread = GetThreadObj();
		InsertSocketObj(pThread, pSocket);
		::CreateThread(NULL, 0, ServerThread, pThread, 0, NULL);
	}
	::LeaveCriticalSection(&g_cs);

	// ָʾ�߳��ؽ��������
	::WSASetEvent(pThread->events[0]);
}

// �Ӹ����̵߳��׽��ֶ����б����Ƴ�һ���׽��ֶ���
void RemoveSocketObj(PTHREAD_OBJ pThread, PSOCKET_OBJ pSocket)
{
	::EnterCriticalSection(&pThread->cs);

	// ���׽��ֶ����б��в���ָ�����׽��ֶ����ҵ���֮�Ƴ�
	PSOCKET_OBJ pTest = pThread->pSockHeader;
	if (pTest == pSocket)
	{
		if (pThread->pSockHeader == pThread->pSockTail)
			pThread->pSockTail = pThread->pSockHeader = pTest->pNext;
		else
			pThread->pSockHeader = pTest->pNext;
	}
	else
	{
		while (pTest != NULL && pTest->pNext != pSocket)
			pTest = pTest->pNext;
		if (pTest != NULL)
		{
			if (pThread->pSockTail == pSocket)
				pThread->pSockTail = pTest;
			pTest->pNext = pSocket->pNext;
		}
	}
	pThread->nSocketCount--;

	::LeaveCriticalSection(&pThread->cs);

	// ָʾ�߳��ؽ��������
	::WSASetEvent(pThread->events[0]);

	// ˵��һ�������ж�
	::InterlockedDecrement(&g_nCurrentConnections);
	//�ص������̣߳���֪�пͻ�ʧȥIO����
	if (g_IoStatusCallback != NULL)
	{
		g_IoStatusCallback(1, NULL);
	}
}

// ��ȡϵͳ�ĵ�ǰʱ�䣬��λ΢��(us)
int64_t GetSysTimeMicros()
{
#ifdef _WIN32
	// ��1601��1��1��0:0:0:000��1970��1��1��0:0:0:000��ʱ��(��λ100ns)
#define EPOCHFILETIME   (116444736000000000UL)
	FILETIME ft;
	LARGE_INTEGER li;
	int64_t tt = 0;
	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;
	// ��1970��1��1��0:0:0:000�����ڵ�΢����(UTCʱ��)
	tt = (li.QuadPart - EPOCHFILETIME) / 10;
	return tt;
#else
	timeval tv;
	gettimeofday(&tv, 0);
	return (int64_t)tv.tv_sec * 1000000 + (int64_t)tv.tv_usec;
#endif // _WIN32
	return 0;
}

/*
//I/O �����Ϣ
public static final int TYPE_MOUSE_MOVE = 0;
public static final int TYPE_MOUSE_LEFT_DOWN = 1;
public static final int TYPE_MOUSE_LEFT_UP = 2;
public static final int TYPE_MOUSE_RIGHT_DOWN = 3;
public static final int TYPE_MOUSE_RIGHT_UP = 4;
public static final int TYPE_MOUSE_WHEEL = 5;
public static final int TYPE_KEY_DOWN = 6;
public static final int TYPE_KEY_UP = 7;
public static final int TYPE_MSG_HB = 8;//����
// mouse Model�����Ϣ
public static final int TYPE_MODEL_NOMAL = 100;
public static final int TYPE_MODEL_GAME = 101;

//����ҵ����Ϣ
public static final int TYPE_BUINSESS_SERVERTYPE = 1000;//����ָ������
public static final int TYPE_BUINSESS_GETTOKEN = 1001;//�����ȡuserid��token
public static final int TYPE_BUINSESS_GETTOKEN_Rsps = 2001;//����userid��token
// login logout
public static final int TYPE_BUINSESS_LOGIN = 2100;
public static final int TYPE_BUINSESS_LOGOUT = 2101;
public static final int TYPE_BUINSESS_LOGOFF = 2102;//[20180712]ע�� �ػ� ���������
public static final int TYPE_BUINSESS_SHUTDOWN = 2103;
public static final int TYPE_BUINSESS_REBOOT = 2104;
//test
public static final int TYPE_TEST_LOOP_BETWEEN_SERVER_AND_CLIENT = 2200
*/
//void DoKeyEvent(char szVal, int nKeyType)
//{
//	/*	if (szVal >= 0x41 && szVal <= 0x5A)
//	{
//	if (nKeyType == 6)
//	{
//	HWND wnd;//���ھ��
//	wnd = GetForegroundWindow();//��õ�ǰ����Ĵ��ھ��
//	DWORD SelfThreadId = GetCurrentThreadId();//��ȡ������߳�ID
//	DWORD ForeThreadId = GetWindowThreadProcessId(wnd, NULL);//���ݴ��ھ����ȡ�߳�ID
//	AttachThreadInput(ForeThreadId, SelfThreadId, true);//�����߳�
//	wnd = GetFocus();//��ȡ�������뽹��Ĵ��ھ��
//	AttachThreadInput(ForeThreadId, SelfThreadId, false);//ȡ�����ӵ��߳�
//	PostMessage(wnd, WM_CHAR, WPARAM(szVal & 0xFF), 0);//����һ������Ϣ
//	}
//	}
//	else*/
//	{
//		if (nKeyType == 6)
//		{
//			keybd_event(szVal, MapVirtualKey(szVal, 0), 0, 0);
//		}
//		if (nKeyType == 7)
//		{
//			keybd_event(szVal, MapVirtualKey(szVal, 0), KEYEVENTF_KEYUP, 0);
//		}
//	}
//}
//
//long g_lTime = 0;
//void DealMouseAndKeyboard(string strContent)
//{
//	int nPos = strContent.find("{");
//	if (nPos<0)
//	{
//		LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "not find { :" << strContent.c_str());
//	}
//	string strExePath = GetExeFileExistDir() + "\\Server.ini";
//	char *strConfigFilePath = (char*)strExePath.c_str();
//	char szPN[20];
//	memset(szPN, 0, 20);
//	char szprocess[150];
//	memset(szprocess, 0, 150);
//
//	while (nPos > -1)
//	{
//		int nPosRight = strContent.find("}");
//		if (nPosRight > 0)
//		{
//			string strMsg = strContent.substr(nPos, nPosRight - nPos);
//			strContent = strContent.substr(nPosRight + 1, strContent.length() - nPosRight - 1);
//			nPos = strContent.find("{");
//			int nMsgPos = strMsg.find("type\":");
//			if (nMsgPos > 0)
//			{
//				
//				int nMsgPosRight = strMsg.find(",");
//				string strType = strMsg.substr(nMsgPos + 6, nMsgPosRight - nMsgPos - 6);
////				LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "type:" << strType.c_str() << " msg:" << strMsg.c_str());
//				int nType = atoi(strType.c_str());
//				if (nType == 100)
//				{
//					if (g_IoStatusCallback != NULL)
//					{
//						g_IoStatusCallback(nType, NULL);
//					}
//					g_nMouseModel = 0;
//					break;
//				}
//				if (nType==101)
//				{
//					if (g_IoStatusCallback != NULL)
//					{
//						g_IoStatusCallback(nType, NULL);
//					}
//					g_nMouseModel = 1;
//					break;
//				}
//				if (nType == 0 || nType>4)
//				{
//					nMsgPos = strMsg.find("[");
//					if (nMsgPos > 0)
//					{
//						nPosRight = strMsg.find("]");
//						if (nPosRight < 0)
//						{
//							LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "not find ] :" << strContent.c_str());
//							break;
//						}
//						strMsg = strMsg.substr(nMsgPos + 1, nPosRight - nMsgPos - 1);
//
//						nMsgPos = strMsg.find(",");
//						string strX = strMsg.substr(0, nMsgPos);
//						strMsg = strMsg.substr(nMsgPos + 1, strMsg.length() - nMsgPos - 1);
//						nMsgPos = strMsg.find(",");
//						string strY = strMsg.substr(0, nMsgPos); 
//						string strExtern = strMsg.substr(nMsgPos + 1, strMsg.length() - nMsgPos - 1);
//						int nX = atoi(strX.c_str());
//						int nY = atoi(strY.c_str());
//						int nExtern = atoi(strExtern.c_str());
//
//						int nUserId = 0;
//						int nToken = 0;
//						//LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "Move x:" << nX << " y:"<<nY);
//
//						//����ƶ��ͼ���
////						INPUT input[1];
//						char szV = nX;
//						switch (nType)
//						{
//						case 0:
//							{//SetCursorPos(nX, nY);
//								  if (g_nMouseModel == 0)
//								  {
//									mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, nX * 65535 / nWidth, nY * 65535 / nHeight, 0, 0);
//								  }
//								  else
//								  {
//									  //����ƫ����
//									  //float fFactor = 1.0f;
//									  //
//									  //if (nExtern != 0)
//									  //{
//										 // fFactor = (float)MOUSE_RATE_BASE / (float)nExtern;;
//										 // if (fFactor > 1)
//										 // {
//											//  fFactor *= MOUSE_RATE;
//										 // }
//										 // if (fFactor < 1)
//										 // {
//											// // fFactor /= MOUSE_RATE;
//										 // }
//									  //}
//
//									  mouse_event(MOUSEEVENTF_MOVE, nX, nY, 0, 0);
//
//								  }
//							}			
//							break;
//						case 5:
//							//����
//							if (nX == 0)
//							{
//								mouse_event(MOUSEEVENTF_WHEEL, 0, 0, 0-100, 0);
//							}
//							else
//							{
//								mouse_event(MOUSEEVENTF_WHEEL, 0, 0, 100, 0);
//							}
//							break;
//						case 6:
//							DoKeyEvent(szV, 6);
//							//LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "keydown value:" << nX);
//							/*memset(input, 0, sizeof(input));
//							input[0].ki.wVk = szV;
//							input[0].type = INPUT_KEYBOARD;
//							SendInput(1, input, sizeof(INPUT));*/
//							break;
//						case 7:
//							DoKeyEvent(szV, 7);
//							//LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "keyup value:" << nX);
//							/*memset(input, 0, sizeof(input));
//							input[0].ki.wVk = 0;
//							input[0].type = INPUT_KEYBOARD;
//							input[0].ki.dwFlags = KEYEVENTF_KEYUP;
//							SendInput(1, input, sizeof(INPUT));*/
//							break;
//						case 8://����//huqb 20180522
//							//CMsgQueue::getInstance().pushSendMsgToQueue("received hb!");
//							break;
//						case 9: //У��ֱ���
//							{
//								LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "recv change resolution msg,resultion:" << nX);
//								switch (nX)
//								{
//								case 0:
//									nWidth = 1280;
//									nHeight = 720;
//									break;
//								case 1:
//									nWidth = 1920;
//									nHeight = 1080;
//									break;
//								default:
//									nWidth = 1280;
//									nHeight = 720;
//									break;
//								}
//								int currentWidth = GetSystemMetrics(SM_CXSCREEN);
//								int currentHeight = GetSystemMetrics(SM_CYSCREEN);
//								if (nWidth != currentWidth&&nHeight != currentHeight)
//								{
//									DEVMODE lpDevMode;
//									lpDevMode.dmBitsPerPel = 32;
//									lpDevMode.dmPelsWidth = nWidth;
//									lpDevMode.dmPelsHeight = nHeight;
//									lpDevMode.dmSize = sizeof(lpDevMode);
//									lpDevMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
//									LONG result;
//									result = ChangeDisplaySettings(&lpDevMode, 0);
//									if (result == DISP_CHANGE_SUCCESSFUL)
//									{
//										ChangeDisplaySettings(&lpDevMode, CDS_UPDATEREGISTRY);
//										LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "change resolution success:" << nX << "old wh:" << currentWidth << "," << currentHeight << ". new wh:" << nWidth << "," << nHeight << ".");
//									}
//									else
//									{
//										ChangeDisplaySettings(NULL, 0);
//										LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "change resolution failed:" << nX << "old wh:" << currentWidth << "," << currentHeight << ". new wh:" << nWidth << "," << nHeight << ".");
//
//									}
//								}
//							}
//							break;
//						case 1000://������������
//							//int nProgress = nX;
//							sprintf(szPN, "p_%d", nX);
//							GetPrivateProfileString("info", szPN, NULL, szprocess, 150, strConfigFilePath);
//							ShellExecute(NULL, "open", szprocess, NULL, NULL, SW_SHOWNORMAL);
//							//switch (nX)//progress id
//							//{
//							//case 0:
//							//	ShellExecute(NULL, "open", "C:\\Program Files\\��Ѷ��Ϸ\\Ӣ������\\TCLS\\Client.exe", NULL, NULL, SW_SHOWNORMAL);
//							//	break;
//							//case 1:
//							//	ShellExecute(NULL, "open", "C:\\Program Files (x86)\\Steam\\Steam.exe", NULL, NULL, SW_SHOWNORMAL);
//							//	break;
//							//case 2:
//							//	//ShellExecute(NULL, "open", "C:\\Program Files (x86)\\Tencent\\QQ\\Bin\\QQScLauncher.exe", NULL, NULL, SW_SHOW);
//							//	break;
//							//}
//							LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "Client request start progress:" << nX);
//							break;
//						case 1001://��ȡ�û�ID��Token
//							
//							/*g_cVideoEnginesIns.GetUseridAndToken(nUserId, nToken);
//							char szBuffer[200];
//							memset(szBuffer, 0, 200);
//							sprintf(szBuffer, "{\"type\":%d,\"value\":[\"id\":%d,\"token\":%d]}", 2001, nUserId, nToken);
//							CMsgQueue::getInstance().pushSendMsgToQueue(szBuffer);
//							LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "Client request id:" << szBuffer);*/
//							break;
//						case 2101:
//							if (g_IoStatusCallback != NULL)
//							{
//								g_IoStatusCallback(3, 0);
//							}
//							break;
//						case 2102://logoff
//						{
//							LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "recv logoff cmd." );
//							//ReSetWindows(EWX_LOGOFF, TRUE);
//						}
//							break;
//						case 2103://shutdown
//						{
//							LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "recv shutdown cmd.");
//	//						ReSetWindows(EWX_SHUTDOWN, TRUE);
//						}
//							break;
//						case 2104://reboot
//						{
//							LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "recv reboot cmd.");
//							ReSetWindows(EWX_REBOOT, TRUE);
//						}
//							break;
//						case 2200://test ��·
//						{
//							SYSTEMTIME timeSys;
//							GetLocalTime(&timeSys);
//							long lTime = timeSys.wMilliseconds + timeSys.wSecond * 1000 + timeSys.wMinute * 60000;
//							LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "recv cycle msg: send time:" << g_lTime << " current time:" << lTime << " diff:" << (lTime - g_lTime));
//						}
//							break;
//						}
//					}
//					else
//					{
//						LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "not find [ :" << strContent.c_str());
//					}
//				}
//				else
//				{//����¼�
//					switch (nType)
//					{
//					case 1:
//						{
//							//long l1 = GetTickCount();
//							mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
//							//long l2 = GetTickCount();
//							SYSTEMTIME timeSys;
//							GetLocalTime(&timeSys);
//							g_lTime = timeSys.wMilliseconds + timeSys.wSecond * 1000 + timeSys.wMinute * 60000;
//							CMsgQueue::getInstance().pushSendMsgToQueue("test send msg:server2client!");
//						/*	long l3 = GetTickCount();
//							printf("\n2-1:%d,3-2:%d,3-1:%d\n", l2 - l1, l3 - l2, l3 - l1);*/
//						}
//						break;
//					case 2:
//						mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
//						break;
//					case 3:
//						mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
//						break;
//					case 4:
//						mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
//						//test send msg:
//						//CMsgQueue::getInstance().pushSendMsgToQueue("test send msg:server2client!");
//						break;
//					}
//				}
//			}
//			else
//			{
//				LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "not find type\\ :" << strContent.c_str());
//			}
//		}
//		else
//		{
//			nPos = -1;
//			LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "not find } :" << strContent.c_str());
//		}
//	}
//}

void NetHeartBeatThread(void* pParam)
{
	PSOCKET_OBJ pSocket = (PSOCKET_OBJ)pParam;
	
	while (true)
	{
		if (pSocket==NULL)
		{
			break;
		}
		long lCurTime = ::GetTickCount();
		//2s��ʱ����
		long lTimeSpace = lCurTime - pSocket->pHeartInfo->lLastTime;
		if (lTimeSpace > 1000)
		{
			RemoveSocketObj((PTHREAD_OBJ)pSocket->pHeartInfo->pThread, pSocket);
			FreeSocketObj(pSocket);
			LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "Client:"<< inet_ntoa(pSocket->addrRemote.sin_addr) << " ������ʱ:" << lTimeSpace<<"ms,�Ͽ����ӡ�");
			break;
		}
//		LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "Client:" << inet_ntoa(pSocket->addrRemote.sin_addr) << " ������:" << lTimeSpace<<" ms��");
		Sleep(500);
	}
}

void NetEventDealThread(void* pParam)
{
	while (true)
	{
		string strMsg = CMsgQueue::getInstance().getRecvMsgFromQueue();
		if (!strMsg.empty())
		{
			int nNetType = DealMouseAndKeyboard(strMsg);//huqb
			if (nNetType == 100 || nNetType == 101 || nNetType == 2101)
			{
				if (g_IoStatusCallback != NULL)
				{
					g_IoStatusCallback(nNetType, NULL);
				}
			}
		}
		else
		{
			Sleep(5);
		}
	}
}


BOOL HandleIO(PTHREAD_OBJ pThread, PSOCKET_OBJ pSocket)
{
	// ��ȡ���巢���������¼�
	WSANETWORKEVENTS event;
	::WSAEnumNetworkEvents(pSocket->s, pSocket->event, &event);
	do
	{
		if (event.lNetworkEvents & FD_READ)			// �׽��ֿɶ�
		{
			if (event.iErrorCode[FD_READ_BIT] == 0)
			{
				char szHead[4];
				char szText[256];
				memset(szText, 0, 256);
				int nRecv = ::recv(pSocket->s, szHead, 4, 0);
				if (nRecv == 4)
				{
					int nBodyLen = (int)szHead[3];
					nRecv = ::recv(pSocket->s, szText, nBodyLen, 0);
					if (nRecv > 0)
					{
						//huqb
						string strContent = szText;
						CMsgQueue::getInstance().pushRecvMsgToQueue(strContent);
						

						//send(pSocket->s, szText, 4, 0);


						//test send msg:
						//CMsgQueue::getInstance().pushSendMsgToQueue("test send msg:server2client!");
					}
				}
				//��¼���� ������Ϣ����Ϊ����������ҵ����Ϣ+������Ϣ)
				if (pSocket->pHeartInfo!=NULL)
				{
					pSocket->pHeartInfo->lLastTime = ::GetTickCount();
				}
			}
			else
			{
				break;
			}
			//huqb 20180522
			if (!pSocket->bBeginHeatbeat)
			{
				pSocket->pHeartInfo = new NETHEARTBEATINFO;
				pSocket->pHeartInfo->pThread = pThread;
				pSocket->pHeartInfo->lLastTime = ::GetTickCount();
				_beginthread(NetHeartBeatThread, 0, (void*)pSocket);
				pSocket->bBeginHeatbeat = true;
			}
		}
		else if (event.lNetworkEvents & FD_CLOSE)	// �׽��ֹر�
		{
			break;
		}
		else if (event.lNetworkEvents & FD_WRITE)	// �׽��ֿ�д
		{
			if (event.iErrorCode[FD_WRITE_BIT] == 0)
			{
				//�����������ʹ����ֽ�
				pSocket->bSendTag = true;
			}
			else
			{
				break;
			}
		}
		//send msg to client
		if (pSocket->bSendTag)
		{
			PMsgInfo pSendMsg = CMsgQueue::getInstance().getSendMsgFromQueue();
			if (pSendMsg!=NULL)
			{
				::send(pSocket->s, pSendMsg->szBuffer,pSendMsg->nBodyLen, 0);//huqb
			}
			delete pSendMsg;
			pSendMsg = NULL;
		}
		return TRUE;
	} while (FALSE);

	// �׽��ֹرգ������д����������򶼻�ת��������ִ��
	//RemoveSocketObj(pThread, pSocket);
	//FreeSocketObj(pSocket);

	return FALSE;
}

PSOCKET_OBJ FindSocketObj(PTHREAD_OBJ pThread, int nIndex) // nIndex��1��ʼ
{
	// ���׽����б��в���
	PSOCKET_OBJ pSocket = pThread->pSockHeader;
	while (--nIndex)
	{
		if (pSocket == NULL)
			return NULL;
		pSocket = pSocket->pNext;
	}
	return pSocket;
}

DWORD WINAPI ServerThread(LPVOID lpParam)
{
	// ȡ�ñ��̶߳����ָ��
	PTHREAD_OBJ pThread = (PTHREAD_OBJ)lpParam;
	while (TRUE)
	{
		//	�ȴ������¼�
		int nIndex = ::WSAWaitForMultipleEvents(
			pThread->nSocketCount + 1, pThread->events, FALSE, WSA_INFINITE, FALSE);
		nIndex = nIndex - WSA_WAIT_EVENT_0;
		// �鿴���ŵ��¼�����
		for (int i = nIndex; i<pThread->nSocketCount + 1; i++)
		{
			nIndex = ::WSAWaitForMultipleEvents(1, &pThread->events[i], TRUE, 1000, FALSE);
			if (nIndex == WSA_WAIT_FAILED || nIndex == WSA_WAIT_TIMEOUT)
			{
				continue;
			}
			else
			{
				if (i == 0)				// events[0]���ţ��ؽ�����
				{
					RebuildArray(pThread);
					// ���û�пͻ�I/OҪ�����ˣ����߳��˳�
					if (pThread->nSocketCount == 0)
					{
						FreeThreadObj(pThread);
						return 0;
					}
					::WSAResetEvent(pThread->events[0]);
				}
				else					// ���������¼�
				{
					// ���Ҷ�Ӧ���׽��ֶ���ָ�룬����HandleIO���������¼�
					PSOCKET_OBJ pSocket = (PSOCKET_OBJ)FindSocketObj(pThread, i);
					if (pSocket != NULL)
					{
						if (!HandleIO(pThread, pSocket))
							RebuildArray(pThread);
					}
					else
						printf(" Unable to find socket object \n ");
				}
			}
		}
	}
	return 0;
}

void SendClientThread(void* p)
{
	//int nX = 300;
	//int nY = 500;
	//while (true)
	//{
	//	mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, nX * 65535 / nWidth, nY * 65535 / nHeight, 0, 0);
	//	nX+=5;
	//	if (nX >= 1000)
	//	{
	//		nX = 300;
	//	}
	//	Sleep(5);
	//}
}

int IoDomain(OnEventUserIOStatus pStatusCb)
{
	//���������߳�
	_beginthread(SendClientThread, 0, NULL);
	//����ҵ����մ����߳�
	_beginthread(NetEventDealThread, 0, NULL);
	USHORT nPort = 8627;	// �˷����������Ķ˿ں�
	g_IoStatusCallback = pStatusCb;
	// ���������׽���
	SOCKET sListen = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(nPort);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if(::bind(sListen, (sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf(" Failed bind() \n");
		return -1;
	}
	::listen(sListen, 200);

	// �����¼����󣬲��������������׽���
	WSAEVENT event = ::WSACreateEvent();
	::WSAEventSelect(sListen, event, FD_ACCEPT|FD_CLOSE);

	::InitializeCriticalSection(&g_cs);

	//init
	//nWidth = GetSystemMetrics(SM_CXSCREEN);
	//nHeight = GetSystemMetrics(SM_CYSCREEN);

	// ����ͻ��������󣬴�ӡ״̬��Ϣ
	while(TRUE)
	{
		int nRet = ::WaitForSingleObject(event, 5*1000);
		if(nRet == WAIT_FAILED)
		{
			printf(" Failed WaitForSingleObject() \n");
			break;
		}
		else if(nRet == WSA_WAIT_TIMEOUT)	// ��ʱ��ʽ״̬��Ϣ
		{
			printf(" \n");
			printf("   TatolConnections: %d \n", g_nTatolConnections);
			printf(" CurrentConnections: %d \n", g_nCurrentConnections);
			continue;
		}
		else								// ���µ�����δ��
		{
			::ResetEvent(event);
			// ѭ����������δ������������
			while(TRUE)
			{
				sockaddr_in si;
				int nLen = sizeof(si);
				SOCKET sNew = ::accept(sListen, (sockaddr*)&si, &nLen);
				if(sNew == SOCKET_ERROR)
					break;
				PSOCKET_OBJ pSocket = GetSocketObj(sNew);
				pSocket->addrRemote = si;
				::WSAEventSelect(pSocket->s, pSocket->event, FD_READ|FD_CLOSE|FD_WRITE);
				AssignToFreeThread(pSocket);
			}
		}
	}
	::DeleteCriticalSection(&g_cs);
	return 0;
}

