
// RemoteDesktopClientDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "RemoteDesktopClient.h"
#include "RemoteDesktopClientDlg.h"
#include "afxdialogex.h"
#include "MutexImp_Win32.h"
#include "MsgTips.h"
#include "Util.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define CLINET_WIDTH_VALUE   1920.0f
#define CLINET_HEIGHT_VALUE  1080.0f


long g_lSendTime = 0;
long g_nCountTime = 0;
long g_nSendCount = 0;
long g_nLastHeartbeat = 0;
CS::CMutexImp   g_Mutex;

HHOOK g_HookHwnd = NULL;
bool  g_bHookLocalKeyMsg = false;

// �����ӳ�
LRESULT CALLBACK MyHookFun(int nCode, WPARAM wParam, LPARAM lParam)
{
	PKBDLLHOOKSTRUCT pVirKey = (PKBDLLHOOKSTRUCT)lParam;
	if (nCode >= 0)
	{
		CRemoteDesktopClientDlg* pThis = CWndsManger::getWndsMangerInstance().getMainDlg();
		SendMessage(pThis->m_hWnd, WM_KEYBOARD_MSG, wParam, lParam);

		if (g_bHookLocalKeyMsg)
		{
			return 1;
		}		

	}
	return CallNextHookEx(g_HookHwnd, nCode, wParam, lParam);
}

void DataRecvCallback(char* szBuffer, int nSize)
{
	//long lCurrent = ::GetTickCount();
	//CString strLog;
	//strLog.Format("recv data:%d,%s,delay:%d\n", nSize, szBuffer, (lCurrent - g_lSendTime));
	//OutputDebugString(strLog);
	//g_nCountTime += (lCurrent - g_lSendTime);
	//if (g_nSendCount > 0 && (g_nSendCount % 10) == 0)
	//{
	//	LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "net avg delay:" << (g_nCountTime / g_nSendCount) << "ms" << " current times:" << g_nSendCount);
	//}
	CRemoteDesktopClientDlg* pThis = CWndsManger::getWndsMangerInstance().getMainDlg();
	int nLen = -1;
	int nType = -1;
	memcpy(&nLen, szBuffer, 4);
	memcpy(&nType, szBuffer+4, 4);
	CString strLog;
	strLog.Format("recv data,len:%d, type:%d\n", nLen, nType);
//	OutputDebugString(strLog);
	if (nType == 1)
	{
		static int nCount = 0;
		FILE* p = NULL;
		char sz[50];
		sprintf_s(sz, "%d.rgb", nCount++);
		fopen_s(&p, sz, "wb");
		if (p)
		{
			fwrite(szBuffer + 8, 1, nLen - 4, p);
			fclose(p);
		}		
		
		if (pThis != NULL)
		{
			if (pThis->m_bMouseRevel)//��ͨģʽ��FPS��Ϸģʽ 
			{
				char* sz = szBuffer + 8;
				char szTemp[32 * 32 * 4];
				for (int n = 0; n < 32; n++)
				{
					memcpy(szTemp + (31 - n) * 128, sz + n * 128, 128);
				}
				CClientDC dc(pThis);
				HBITMAP hBmp = pThis->MakeBitmap(dc, (LPBYTE)(szTemp), 32, 32, 32);
				g_Mutex.Lock();
				pThis->m_hCursor = pThis->CreateCursorFromBitmap(hBmp, RGB(0, 0, 0), 0, 0);
				g_Mutex.Unlock();
				DeleteObject(hBmp);
			}
		}
	}
	if (nType == 0)//����
	{
	}
	if (nType == TYPE_BUINSESS_RECONNECTMEDIA)
	{
		LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "[huqb]meidia restarted. recv recoonect cmd.");
		ReStartRtsp();
	}
	g_nLastHeartbeat = ::GetTickCount();
}

void PlaystatusCallback(int nStatus)
{
	char sz[100];
	sprintf_s(sz, "recv status cb: %d\n", nStatus);
//	OutputDebugString(sz);
	CRemoteDesktopClientDlg* pThis = CWndsManger::getWndsMangerInstance().getMainDlg();
	static int nPlayerStatus = TYPE_STATUS_PLAY_OK;
	static int nReConnCount = 0;
	if (nStatus == 4)
	{
		nPlayerStatus = TYPE_STATUS_PLAY_OK;
	}
	else if (nStatus == 6)
	{
		if (nPlayerStatus != TYPE_STATUS_PLAY_CONNECTING)
		{//����
			if (pThis != NULL)
			{
				ReStartRtsp();
				LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "[huqb] nStatus == 6 ReStartRtsp.");
			}
		}
		else
		{
			nReConnCount++;
			if (nReConnCount >= 3)
			{
				nPlayerStatus = TYPE_STATUS_PLAY_ERROR;
				nReConnCount = 0;
				return;
			}
			
		}
		nPlayerStatus = TYPE_STATUS_PLAY_CONNECTING;
	}
	else if (nStatus == 3)
	{
		nPlayerStatus = TYPE_STATUS_PLAY_CLOSE;
	}
	else if (nStatus == 1)
	{
		nPlayerStatus = TYPE_STATUS_PLAY_CLOSE;
	}
	else if (nStatus == 7)//rtsp��ͨ
	{
		CString strMsg = "";
		strMsg = pThis->createMsgBody(TYPE_BUINSESS_REBOOTMEDIA, pThis->m_nResolution, 0, 0);
		pThis->sendMsg(strMsg);
		LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "[huqb]meidia display failed.send restart cmd.");
	}
	else//test cycle time:
	{

		CString strMsg = pThis->createMsgBody(TYPE_TEST_LOOP_BETWEEN_SERVER_AND_CLIENT, nStatus, 0, 0);

		pThis->sendMsg(strMsg);
		//CString str;
		//str.Format("recv frame,time:%d", nStatus);
		//LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), str);
	}
}

void RunStatusThread(void* pParam)
{
	CRemoteDesktopClientDlg* pThis = (CRemoteDesktopClientDlg*)pParam;
	if (pThis)
	{
		while (!pThis->m_bExitThread)
		{
			int nStatus = pThis->GetRunTimeStatus();
			switch (nStatus)
			{
			case TYPE_STATUS_NET_LOST:
				CMsgTips::getMsgTipsInstance().SetTipText("���粻�ȶ�����������...");
				pThis->ConnectServer();
				LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "Current status :TYPE_STATUS_NET_LOST");
				break;
			case TYPE_STATUS_PLAY_ERROR:
				CMsgTips::getMsgTipsInstance().SetTipText("���Ŵ���.");
				LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "Current status :TYPE_STATUS_PLAY_ERROR");
				break;
			case TYPE_STATUS_PLAY_CONNECTING:
				CMsgTips::getMsgTipsInstance().SetTipText("���粻�ȶ�.������...");
				LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "Current status :TYPE_STATUS_PLAY_CONNECTING");
				break;
			default:
				;
			}
			Sleep(500);
		}
	}
}

void NetHeartbeatThread(void* pParam)
{
	CRemoteDesktopClientDlg* pThis = (CRemoteDesktopClientDlg*)pParam;
	if (pThis)
	{
		g_nLastHeartbeat = ::GetTickCount();
		//�ֱ���ƥ��
		CString strMsg = "";
		if (pThis->m_nResolution != 2)
		{
			strMsg = pThis->createMsgBody(TYPE_MSG_FBL, pThis->m_nResolution, 0, 0);
		}
		pThis->sendMsg(strMsg);
		//������Ӧ����Ϸ
		int nGame = pThis->GetStartProcessID();
		strMsg = pThis->createMsgBody(TYPE_BUINSESS_SERVERTYPE, nGame, 0,0);
		pThis->sendMsg(strMsg);
		while (!pThis->m_bExitThread)
		{
			Sleep(500);

			strMsg = pThis->createMsgBody(TYPE_MSG_HB, 0, 0, 0);
			g_lSendTime = ::GetTickCount();
			g_nSendCount++;
			int nRet = pThis->sendMsg(strMsg);
			if (nRet < 0)
			{
				char sz[100];
				sprintf_s(sz, "heartbeat result: %d\n", nRet);
				OutputDebugString(sz);

				//pThis->SetRunTimeStatus(TYPE_STATUS_NET_LOST);
			}
			
			long lCurrent = ::GetTickCount();
			if (lCurrent -g_nLastHeartbeat>3500)
			{
				pThis->SetRunTimeStatus(TYPE_STATUS_NET_LOST);
			} 
			else
			{

				if (pThis->GetRunTimeStatus() == TYPE_STATUS_NET_LOST)
				{
					pThis->SetRunTimeStatus(TYPE_STATUS_NET_OK);
				}
			}
		}
	}

}

void MouseMoveEventThread(void* pParam)
{
	CRemoteDesktopClientDlg * pThis = (CRemoteDesktopClientDlg*)pParam;
	while (!pThis->m_bExitThread)
	{
		int x = 0;
		int y = 0;
		if (!pThis->m_bMouseRevel)
		{
			g_Mutex.Lock();
			x = pThis->m_nBaseX;
			y = pThis->m_nBaseY;
			pThis->m_nBaseX = 0;
			pThis->m_nBaseY = 0;
			g_Mutex.Unlock();

			if (x != 0 || y != 0)
			{
				CRect rect;
				GetClientRect(pThis->m_hWnd, &rect);
				int nCurX = rect.Width();
				int nCurY = rect.Height();
				float fRateX = pThis->m_nWidth / (float)nCurX;
				float fRateY = pThis->m_nHeight / (float)nCurY;
				float fX = (float)x*fRateX;
				float fY = (float)y*fRateY;

				CString strMsg = pThis->createMsgBody(TYPE_MOUSE_MOVE, (int)fX, (int)fY, 0);
				if (g_bHookLocalKeyMsg)
				{
					pThis->m_cNet.SendBuffer(strMsg);
				}
			}
		}
		else
		{

			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(pThis->m_hWnd, &pt);

			if (pThis->m_nLastX != pt.x || pThis->m_nLastY != pt.y)
			{
				CRect rect;
				GetClientRect(pThis->m_hWnd, &rect);
				int nCurX = rect.Width();
				int nCurY = rect.Height();
				float fRateX = pThis->m_nWidth / (float)nCurX;
				float fRateY = pThis->m_nHeight / (float)nCurY;
				float fX = (float)pt.x*fRateX;
				float fY = (float)pt.y*fRateY;

				g_lSendTime = ::GetTickCount();
				g_nSendCount++;

				CString strMsg = pThis->createMsgBody(TYPE_MOUSE_MOVE, (int)fX, (int)fY, 0);
				if (g_bHookLocalKeyMsg)
				{
					pThis->m_cNet.SendBuffer(strMsg);
				}

				//char sz[50];
				//sprintf_s(sz, "x:%d,y:%d. fx:%d,fy:%d\n", pt.x, pt.y, (int)fX, (int)fY);
				//OutputDebugString(sz);
			}

			pThis->m_nLastX = pt.x;
			pThis->m_nLastY = pt.y;
		}

		Sleep(15);
	}
}

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRemoteDesktopClientDlg �Ի���


CRemoteDesktopClientDlg::CRemoteDesktopClientDlg(const CString strServIP, const int nResolution, const int nGameType, CWnd* pParent /*=NULL*/)
	: CDialogEx(CRemoteDesktopClientDlg::IDD, pParent)
	, m_nLastX(0)
	, m_nLastY(0)
	, m_strServIP(strServIP)
	, m_nResolution(nResolution)// 0:1280*720  1:1920*1080
	, m_nGameType(nGameType)
	, m_OldRect(50,50,640,360)
	, m_nBaseX(0)
	, m_nBaseY(0)
	, m_bMouseRevel(false)
	, m_bMouseLeave(false)
	, m_hHandCursor(NULL)
	, m_hCursor(NULL)
	, m_bExitThread(false)
	, m_nRunTimeStatus(TYPE_STATUS_RUN_OK)
	, m_hBmpback(NULL)
	, m_bNetRet(false)
	, m_bF11Enable(false)
	, m_bMouseDown(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	switch (m_nResolution)
	{
	case 0:
		m_nWidth = 1280;
		m_nHeight = 720;
		break;
	case 1:
		m_nWidth = 1920;
		m_nHeight = 1080;
		break;
	default:
		m_nWidth = 1280;
		m_nHeight = 720;
		break;
	}
}

void CRemoteDesktopClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PLAY, m_staPlayer);
}

BEGIN_MESSAGE_MAP(CRemoteDesktopClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR()
	ON_WM_MOUSEMOVE()
	ON_WM_CLOSE()
	ON_MESSAGE(WM_INPUT, OnInput)
	ON_WM_MOUSELEAVE()
	ON_WM_SETCURSOR()
	ON_MESSAGE(WM_KEYBOARD_MSG, OnKybordEvent)
	ON_WM_ACTIVATE()
END_MESSAGE_MAP()


// CRemoteDesktopClientDlg ��Ϣ�������

void RegKeyboardRawInput(HWND hwnd)
{

	RAWINPUTDEVICE Rid[1];

	Rid[0].usUsagePage = 0x01;
	Rid[0].usUsage = 0x02;
	Rid[0].dwFlags = RIDEV_INPUTSINK;   // adds HID mouse and also ignores legacy mouse messages   RIDEV_NOLEGACY RIDEV_INPUTSINK  
	Rid[0].hwndTarget = hwnd;

	//Rid[1].usUsagePage = 0x01;
	//Rid[1].usUsage = 0x06;
	//Rid[1].dwFlags = RIDEV_INPUTSINK;   // adds HID keyboard and also ignores legacy keyboard messages RIDEV_NOLEGACY RIDEV_INPUTSINK  
	//Rid[1].hwndTarget = hwnd;

	if (RegisterRawInputDevices(Rid, /*2*/1, sizeof(Rid[0])) == FALSE) {
		//registration failed. Call GetLastError for the cause of the error  
		//MessageBox(NULL, "ע��raw input ʧ�ܣ�", "ע�� raw input", MB_OK + MB_ICONINFORMATION);
		LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "RegisterRawInputDevices failed.");
	}
}

BOOL CRemoteDesktopClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ


	return InitAll();  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

bool CRemoteDesktopClientDlg::ConnectServer()
{
//	m_cNet.StopNet();
	bool bRet = m_cNet.StartNet(m_strServIP, DataRecvCallback);
	if (!bRet)
	{//tips
		CMsgTips::getMsgTipsInstance().SetTipText("����ʧ�ܣ�������...");
	}
	return bRet;
}

void CRemoteDesktopClientDlg::PlayRemoteDesktop()
{
	//����״̬���ģ��
	_beginthread(RunStatusThread, 0, this);
	//��ʼ������ģ��
	m_bNetRet = ConnectServer();
	if (!m_bNetRet)
	{
		MessageBox("���粻ͨ�����������Ƿ��������߷������Ƿ�����");
		exit(0);
	}
	SetRunTimeStatus(TYPE_STATUS_NET_OK);
	_beginthread(NetHeartbeatThread, 0, this);
	//��ʼ����Ƶ����ģ��
	m_hWndPlayer = GetDlgItem(IDC_STATIC_PLAY)->GetSafeHwnd();

	PlayScreen();

}

BOOL CRemoteDesktopClientDlg::InitAll()
{
	g_HookHwnd = SetWindowsHookEx(WH_KEYBOARD_LL, MyHookFun, AfxGetInstanceHandle(), 0);
//	return TRUE;
	//tips
//	CMsgTips::getMsgTipsInstance().SetTipText("��������...");
	//��ʼ����־
	//�������ú���־
	m_bExitThread = false;
	LogInit("CaptureClient_0");
	LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "Client Start, InitLog......");
	LOG4CPLUS_INFO(Logger::getInstance(ALARM_SERVER_LOG), "vm ip:"<<m_strServIP);

	RegKeyboardRawInput(m_hWnd);//ע��������

	//ShowWindow(SW_MAXIMIZE);
	MoveWindow(0, 0, (int)m_nWidth, (int)m_nHeight);
	CRect rc;
	GetClientRect(&rc);
	CStatic* pPlay = (CStatic*)GetDlgItem(IDC_STATIC_PLAY);
	pPlay->MoveWindow(&rc);

	PlayRemoteDesktop();

	//// TODO:  �ڴ���Ӷ���ĳ�ʼ������
	m_dwStyle = GetStyle();//��ȡ����ʽ 
	m_dwExStyle = GetExStyle();//��ȡ����չ��ʽ 

	POINT point;
	GetCursorPos(&point);

	_beginthread(MouseMoveEventThread, 0, this);

	SetMouseModel();

	GetWindowRect(&m_OldRect);
	ClientToScreen(&m_OldRect);
	
	LPSTR ls = MAKEINTRESOURCEA(IDC_CURSOR1);
	HINSTANCE hInstance = AfxGetInstanceHandle();
	m_hHandCursor = LoadCursor(hInstance, ls);
	m_hCursor = LoadCursor(NULL, IDC_ARROW);
	this->SetWindowText("������");
	setWindowsMode(2);
	CMsgTips::getMsgTipsInstance().SetTipText("�˳��밴F11");

	

	m_staPlayer.ModifyStyle(NULL, SS_BITMAP);
	m_hBmpback = (HBITMAP)::LoadImage(0, _T("back.bmp"), IMAGE_BITMAP, rc.Width(), rc.Height(), LR_LOADFROMFILE);
	m_staPlayer.SetBitmap(m_hBmpback);
	m_staPlayer.ShowWindow(TRUE);
	return TRUE;
}

void CRemoteDesktopClientDlg::ReleaseAll()
{
	//��������� huqb
	//CString strMsg = createMsgBody(TYPE_BUINSESS_REBOOT, (int)0, (int)0, 0);
	//m_cNet.SendBuffer(strMsg);

	if (g_HookHwnd != NULL)
	{
		UnhookWindowsHookEx(g_HookHwnd);
	}
	m_bExitThread = true;
	Sleep(800);
	m_cNet.StopNet();
	StopClient();
//�����ɰ汾��huqb
	//CWndsManger::getWndsMangerInstance().getUserGameDlog()->sendRemoteCloseMsg();
}

int  CRemoteDesktopClientDlg::PlayScreen()
{
	char strUrl[50];
	memset(strUrl, 0, 50);
	sprintf_s(strUrl, "rtsp://%s/live", m_strServIP.GetBuffer(0));

	return StartRtspClientLib(strUrl, (void*)m_hWndPlayer, (int)m_nWidth, (int)m_nHeight, PlaystatusCallback);
}

void CRemoteDesktopClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CRemoteDesktopClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

void CRemoteDesktopClientDlg::setWindowsMode(int nMode)
{
	if (nMode == 0)
	{
		LONG_PTR Style = ::GetWindowLongPtr(m_hWnd, GWL_STYLE) | WS_CAPTION;
		::SetWindowLongPtr(m_hWnd, GWL_STYLE, Style);
		ShowWindow(SW_RESTORE);
		MoveWindow(&m_OldRect);
		m_nDlgSizeType = 0;
	}
	if (nMode == 1)
	{
		//ShowWindow(SW_MINIMIZE);
		SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
		m_nDlgSizeType = 1;
	}
	if (nMode == 2)
	{
		m_dwStyle = GetStyle();//��ȡ����ʽ  
		DWORD dwNewStyle = WS_OVERLAPPED | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
		dwNewStyle &= m_dwStyle;//��λ�뽫����ʽȥ��  
		SetWindowLong(m_hWnd, GWL_STYLE, dwNewStyle);//���ó��µ���ʽ  
		m_dwExStyle = GetExStyle();//��ȡ����չ��ʽ  
		DWORD dwNewExStyle = WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR;
		dwNewExStyle &= m_dwExStyle;//��λ�뽫����չ��ʽȥ��  
		SetWindowLong(m_hWnd, GWL_EXSTYLE, dwNewExStyle);//�����µ���չ��ʽ  
		SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_FRAMECHANGED);

		//ȡ��������
		//::SetWindowLong(m_hWnd, GWL_STYLE, GetWindowLong(m_hWnd, GWL_STYLE)& ~WS_CAPTION);
		//::SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		ShowWindow(SW_SHOWMAXIMIZED);

		m_nDlgSizeType = 2;
	}
}

void CRemoteDesktopClientDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	
	if (nType == 2)
	{
		setWindowsMode(nType);
	}

	CStatic* pPlay = (CStatic*)GetDlgItem(IDC_STATIC_PLAY);
	if (pPlay != NULL)
	{
		CRect rc;
		GetClientRect(&rc);
		pPlay->MoveWindow(rc);
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CRemoteDesktopClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

HBRUSH CRemoteDesktopClientDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����
	//test
	//if (nCtlColor == CTLCOLOR_STATIC)
	//{
	//	pDC->SetTextColor(RGB(255, 255, 255));
	//	pDC->SetBkColor(RGB(0, 0, 255));
	//	return ::CreateSolidBrush(RGB(10, 10, 10));
	//}

	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}

int CRemoteDesktopClientDlg::sendMsg(CString strMsg)
{
	int nRet = -1;
	if (m_bNetRet)
	{
		nRet = m_cNet.SendBuffer(strMsg);
	}
	return nRet;
}

void CRemoteDesktopClientDlg::SetMouseModel()
{
	m_bMouseRevel = !m_bMouseRevel;
	CString strMsg = "";
	if (m_bMouseRevel)
	{
		strMsg = createMsgBody(TYPE_MODEL_NOMAL, (int)0, (int)0, 0);
	}
	else
	{
		strMsg = createMsgBody(TYPE_MODEL_GAME, (int)0, (int)0, 0);
	}
	sendMsg(strMsg);
}

void CRemoteDesktopClientDlg::OnMouseMove(UINT nFlags, CPoint point)
{	

	TRACKMOUSEEVENT   tme;
	tme.cbSize = sizeof(tme);
	tme.hwndTrack = m_hWnd;
	tme.dwFlags = TME_LEAVE;
	_TrackMouseEvent(&tme);

	if (m_bMouseLeave)
	{
		m_bMouseLeave = false;

		//while (ShowCursor(FALSE) >= 0)
		//	ShowCursor(FALSE);

	}

	CDialogEx::OnMouseMove(nFlags, point);
}


void CRemoteDesktopClientDlg::OnClose()
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
	g_bHookLocalKeyMsg = false;
	if (MessageBox("��ȷ��Ҫ�˳�����Ϸ��", "��ܰ��ʾ", MB_ICONEXCLAMATION | MB_OKCANCEL) == IDCANCEL)
	{
		m_bF11Enable = false;
		g_bHookLocalKeyMsg = true;
		return;
	}
	ReleaseAll();
	CDialogEx::OnClose();
}


BOOL CRemoteDesktopClientDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO:  �ڴ����ר�ô����/����û���
	//if (pMsg->message == WM_KEYDOWN)
	//{
	//	if (pMsg->wParam == 'Q')
	//	{
	//		if (::GetKeyState(VK_CONTROL) < 0)
	//		{
	//			if (::GetKeyState(VK_SHIFT) < 0)
	//			{
	//				setWindowsMode(0);
	//			}
	//		}
	//	}
	//	if (pMsg->wParam == 'W')
	//	{
	//		if (::GetKeyState(VK_CONTROL) < 0)
	//		{
	//			if (::GetKeyState(VK_SHIFT) < 0)
	//			{
	//				SetMouseModel();
	//				if (!m_bMouseRevel)
	//				{
	//					m_hCursor = m_hHandCursor;// LoadCursor(NULL, IDC_IBEAM);
	//					CMsgTips::getMsgTipsInstance().SetTipText("����FPS��Ϸģʽ���˳��밴CTRL+SHIFT+W");
	//				}
	//				else
	//				{
	//					m_hCursor = LoadCursor(NULL, IDC_ARROW);
	//					CMsgTips::getMsgTipsInstance().SetTipText("������ͨģʽ������Ϸģʽ�밴CTRL+SHIFT+W");
	//				}
	//			}
	//		}
	//	}
	if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == '\r')
	{
		if (!m_bF11Enable)
			return TRUE;
	}
			

	//	return TRUE;
	//	
	//}

	return CDialogEx::PreTranslateMessage(pMsg);
}

LRESULT CRemoteDesktopClientDlg::OnInput(WPARAM wParam, LPARAM lParam)
{
	UINT dwSize = 0;
	GetRawInputData(
		(HRAWINPUT)lParam,
		(UINT)RID_INPUT,
		(LPVOID)NULL,
		(PUINT)&dwSize,
		(UINT)sizeof(RAWINPUTHEADER)
		);
	LPBYTE lpbBuffer = new BYTE[dwSize];

	GetRawInputData(
		(HRAWINPUT)lParam,
		(UINT)RID_INPUT,
		(LPVOID)lpbBuffer,
		(PUINT)&dwSize,
		(UINT)sizeof(RAWINPUTHEADER)
		);

	RAWINPUT * raw = (RAWINPUT *)lpbBuffer;

	CString strTemp;;
	
	if (raw->header.dwType == RIM_TYPEMOUSE)
	{
		if ((raw->data.mouse.usButtonFlags)!=0)
		{
			strTemp.Format(_T("-���:state:%d, BtnState=%04x deltaX=%d deltaY=%d\n"),
				raw->data.mouse.usButtonFlags,
				raw->data.mouse.ulButtons,
				raw->data.mouse.lLastX,
				raw->data.mouse.lLastY
				);
			//OutputDebugString(strTemp);
		}
		
		int x = raw->data.mouse.lLastX;
		int y = raw->data.mouse.lLastY;
		CString strMsg = "";	

		switch (raw->data.mouse.usButtonFlags)
		{
			case 0:
			{
				  if (x != 0 || y != 0)
				  {
					  g_Mutex.Lock();
					  m_nBaseX += x;
					  m_nBaseY += y;
					  g_Mutex.Unlock();
				  }

			}
			break;
			case RI_MOUSE_LEFT_BUTTON_DOWN:
			{
				g_lSendTime = ::GetTickCount();
			    g_nSendCount++;
				strMsg = createMsgBody(TYPE_MOUSE_LEFT_DOWN, x, y, 0);
				if (g_bHookLocalKeyMsg)
				{
					m_cNet.SendBuffer(strMsg);
				}
			}
			break;
			case RI_MOUSE_LEFT_BUTTON_UP:
			{
				strMsg = createMsgBody(TYPE_MOUSE_LEFT_UP, x, y, 0);
				if (g_bHookLocalKeyMsg)
				{
					m_cNet.SendBuffer(strMsg);
				}
			}
			break;
			case RI_MOUSE_RIGHT_BUTTON_DOWN:
			{
				strMsg = createMsgBody(TYPE_MOUSE_RIGHT_DOWN, x, y, 0);
				if (g_bHookLocalKeyMsg)
				{
					m_bMouseDown = true;
					m_cNet.SendBuffer(strMsg);
				}
				//CMsgTips::getMsgTipsInstance().SetTipText("you��dwon...");
			}
			break;
			case RI_MOUSE_RIGHT_BUTTON_UP:
			{
				strMsg = createMsgBody(TYPE_MOUSE_RIGHT_UP, x, y, 0);
				if (g_bHookLocalKeyMsg)
				{
					m_cNet.SendBuffer(strMsg);
					//OutputDebugString(strMsg);
					//OutputDebugString("\n");
				}
				else
				{
					if (m_bMouseDown)
					{
						m_bMouseDown = false;
						m_cNet.SendBuffer(strMsg);
					}
				}
			}
			break;
			case RI_MOUSE_WHEEL: //Raw input comes from a mouse wheel. The wheel delta is stored in usButtonData.
			{
				int x = raw->data.mouse.usButtonData;
				if (x == 120)
					x = 1;
				else
					x = 0;
				strMsg = createMsgBody(TYPE_MOUSE_WHEEL, x, 0, 0);
				if (g_bHookLocalKeyMsg)
				{
					m_cNet.SendBuffer(strMsg);
				}
			}
				break;
			default:
				break;
		}
			

		delete[] lpbBuffer;
		return TRUE;
	}
	else if (raw->header.dwType == RIM_TYPEKEYBOARD)
	{
		/*CString strMsg = "";
		switch (raw->data.keyboard.Message)
		{
		case WM_KEYDOWN:
		{
			strMsg = createMsgBody(TYPE_KEY_DOWN, (int)raw->data.keyboard.VKey, (int)0, 0);
			m_cNet.SendBuffer(strMsg);
		}
			break;
		case WM_KEYUP:
		{
			strMsg = createMsgBody(TYPE_KEY_UP, (int)raw->data.keyboard.VKey, (int)0, 0);
			m_cNet.SendBuffer(strMsg);
		}
			break;
		}*/

		//OutputDebugString(strTemp);

		delete[] lpbBuffer;
		return TRUE;
	}

	return 0;
}


void CRemoteDesktopClientDlg::OnMouseLeave()
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
	m_bMouseLeave = true;
	while (ShowCursor(TRUE) < 0)
		ShowCursor(TRUE);

	CDialogEx::OnMouseLeave();
}

BOOL CRemoteDesktopClientDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (m_hCursor != NULL)
	{
		::SetCursor(m_hCursor);
		return TRUE;
	}

	return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}

void CRemoteDesktopClientDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialogEx::OnActivate(nState, pWndOther, bMinimized);

	CString strLog;
	strLog.Format("@@@ state:%d,bMin:%d\n", nState, bMinimized);
	OutputDebugString(strLog);

	if (!AllkeysIsUp())
	{
		return;
	}
	// TODO:  �ڴ˴������Ϣ����������
	if (nState == WA_INACTIVE)
	{
//		setEnable(false);
		g_bHookLocalKeyMsg = false;
	}
	else
	{
//		setEnable(true);
		g_bHookLocalKeyMsg = true;
	}

	strLog.Format("### state:%d,bMin:%d\n", nState, bMinimized);
	OutputDebugString(strLog);
}

void CRemoteDesktopClientDlg::Setkystate(int nCode, bool bStatus)
{
	map<int, bool>::iterator it = m_mapKeyStatus.find(nCode);
	if (it == m_mapKeyStatus.end())
	{
		m_mapKeyStatus.insert(std::make_pair(nCode, bStatus));
	}
	else
	{
		it->second = bStatus;
	}
}

bool CRemoteDesktopClientDlg::Getkeystate(int nCode)
{
	bool bStatus = false;
	map<int, bool>::iterator it = m_mapKeyStatus.find(nCode);
	if (it != m_mapKeyStatus.end())
	{
		bStatus = it->second;
	}
	return bStatus;
}

bool CRemoteDesktopClientDlg::AllkeysIsUp()
{
	bool bRet = true;
	map<int, bool>::iterator it = m_mapKeyStatus.begin();
	for ( ; it != m_mapKeyStatus.end(); it++)
	{
		if (it->second)//�м�����down״̬
		{
			bRet = false;
			CString str;
			str.Format("key:%d not up.\n", it->first);
			OutputDebugString(str);
//			break;
		}
	}
	return bRet;
}

void KeyEventThread(void* p)
{
	int nEvnet = (int)p;
	CRemoteDesktopClientDlg* pThis = CWndsManger::getWndsMangerInstance().getMainDlg();
	switch (nEvnet)
	{
	case 1://win+z ϵͳ��������Ϸ
		while (!pThis->AllkeysIsUp())
		{
			Sleep(100);
		}
		g_bHookLocalKeyMsg = true;
		break;
	case 2://win+z ϵͳ��������
		while (!pThis->AllkeysIsUp())
		{
			Sleep(100);
		}
		g_bHookLocalKeyMsg = false;
		break;
	}
}

LRESULT CRemoteDesktopClientDlg::OnKybordEvent(WPARAM wParam, LPARAM lParam)
{
	CString strMsg = "";
	PKBDLLHOOKSTRUCT pVirKey = (PKBDLLHOOKSTRUCT)lParam;
	bool bKeyDown = true;
	bool bLeftCtrlAltDelete = false;
	bool bRightCtrlAltDelete = false;
	int nKeyCode = pVirKey->vkCode;

	CString str;
	str.Format("value:%d,scancode:%d,flag:%d,dwexternInfo:%x\n", pVirKey->vkCode, pVirKey->scanCode, pVirKey->flags, pVirKey->dwExtraInfo);
	OutputDebugString(str);

	//�����������(p->vkCode == VK_TAB) && ((p->flags & LLKHF_ALTDOWN) != 0)
	if (wParam == WM_KEYDOWN||wParam == WM_SYSKEYDOWN)
	{
		{
			Setkystate(nKeyCode, bKeyDown);
		}

		if (pVirKey->vkCode == VK_F8)//'Q'
		{
			if (Getkeystate(VK_LCONTROL) && Getkeystate(VK_LSHIFT))
			{
				if (m_nDlgSizeType==2)
				{
					setWindowsMode(0);
				}
				else
				{
					setWindowsMode(2);
				}
			}
		}
		if (pVirKey->vkCode == 'W')
		{
			if (Getkeystate(VK_LCONTROL) && Getkeystate(VK_LSHIFT))
			{
				SetMouseModel();
				if (!m_bMouseRevel)
				{
					m_hCursor = m_hHandCursor;// LoadCursor(NULL, IDC_IBEAM);
					CMsgTips::getMsgTipsInstance().SetTipText("����FPS��Ϸģʽ���˳��밴CTRL+SHIFT+W");
				}
				else
				{
					m_hCursor = LoadCursor(NULL, IDC_ARROW);
					CMsgTips::getMsgTipsInstance().SetTipText("������ͨģʽ������Ϸģʽ�밴CTRL+SHIFT+W");
				}
			}
		}
		
		if (pVirKey->vkCode == 'Z')
		{
			if (Getkeystate(VK_LWIN))
			{//20180830 
				//if (m_nDlgSizeType == 1)
				//{
				//	setWindowsMode(2);
				//	CMsgTips::getMsgTipsInstance().SetTipText("��������Ϸ�����������밴Win+Z");
				//	_beginthread(KeyEventThread, 0, (void*)1);
				//}
				//else
				//{
				//	setWindowsMode(1);
				//	CMsgTips::getMsgTipsInstance().SetTipText("��С������Ϸ�������밴Win+Z");
				//	_beginthread(KeyEventThread, 0, (void*)2);
				//}
				return S_OK;
			}
		}

		if (pVirKey->vkCode == 'D')
		{
			if (Getkeystate(VK_LWIN))
			{
				return S_OK;
			}
		}
		
		if (pVirKey->vkCode == VK_DELETE)
		{
			if (Getkeystate(VK_LMENU))//alt
			{
				if (Getkeystate(VK_LCONTROL))
				{
					//CMsgTips::getMsgTipsInstance().SetTipText("CTRL+ALT+DEL");
					OutputDebugString("\n L ctr alt delete.\n");
					bLeftCtrlAltDelete = true;
				}
			}
			if (Getkeystate(VK_RMENU))//alt
			{
				if (Getkeystate(VK_RCONTROL))
				{
					//CMsgTips::getMsgTipsInstance().SetTipText("CTRL+ALT+DEL");
					OutputDebugString("\n R ctr alt delete.\n");
					bRightCtrlAltDelete = true;
				}
			}
		}
		if (pVirKey->vkCode == VK_F11)
		{
			if (!m_bF11Enable)
			{
				m_bF11Enable = true;
				PostMessage(WM_CLOSE);
			}
		}

	}
	else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
	{
		bKeyDown = false;
	}

	if (g_bHookLocalKeyMsg)
	{
		if (nKeyCode == VK_LSHIFT)
		{
			nKeyCode = VK_SHIFT;//huqb
		}
		if (nKeyCode == VK_LMENU)
		{
			nKeyCode = VK_MENU;//huqb
		}
		if (wParam == WM_KEYDOWN)
		{
			strMsg = createMsgBody(TYPE_KEY_DOWN, nKeyCode, (int)pVirKey->scanCode, (long)pVirKey->flags);
		}
		if (wParam == WM_KEYUP)
		{
			strMsg = createMsgBody(TYPE_KEY_UP, nKeyCode, (int)pVirKey->scanCode, (long)pVirKey->flags);
		}
		m_cNet.SendBuffer(strMsg);
		CString str;
		str.Format("send:key:%d,status:%d\n", pVirKey->vkCode, wParam);
//		OutputDebugString(str);
	}
	//��λԶ�̻� c,a,d��
	if (bLeftCtrlAltDelete)
	{
		strMsg = createMsgBody(TYPE_KEY_UP, VK_LCONTROL, (int)pVirKey->scanCode, (long)pVirKey->flags);
		m_cNet.SendBuffer(strMsg);
		Setkystate(VK_LCONTROL, false);
		strMsg = createMsgBody(TYPE_KEY_UP, VK_MENU, (int)pVirKey->scanCode, (long)pVirKey->flags);
		m_cNet.SendBuffer(strMsg);
		Setkystate(VK_LMENU, false);
		strMsg = createMsgBody(TYPE_KEY_UP, VK_DELETE, (int)pVirKey->scanCode, (long)pVirKey->flags);
		m_cNet.SendBuffer(strMsg);
		Setkystate(VK_DELETE, false);

		bLeftCtrlAltDelete = false;
	}
	if (bRightCtrlAltDelete)
	{
		strMsg = createMsgBody(TYPE_KEY_UP, VK_RCONTROL, (int)pVirKey->scanCode, (long)pVirKey->flags);
		m_cNet.SendBuffer(strMsg);
		Setkystate(VK_RCONTROL, false);
		strMsg = createMsgBody(TYPE_KEY_UP, VK_RMENU, (int)pVirKey->scanCode, (long)pVirKey->flags);
		m_cNet.SendBuffer(strMsg);
		Setkystate(VK_RMENU, false);
		strMsg = createMsgBody(TYPE_KEY_UP, VK_DELETE, (int)pVirKey->scanCode, (long)pVirKey->flags);
		m_cNet.SendBuffer(strMsg);
		Setkystate(VK_DELETE, false);

		bRightCtrlAltDelete = false;
	}

	Setkystate(pVirKey->vkCode, bKeyDown);
	//����F11��ť
	if (m_bF11Enable)
	{
		strMsg = createMsgBody(TYPE_KEY_UP, VK_F11, (int)pVirKey->scanCode, (long)pVirKey->flags);
		m_cNet.SendBuffer(strMsg);
		Setkystate(VK_F11, false);
	}
	
	//CString str;
	//str.Format("record:key:%d,status:%d\n", pVirKey->vkCode, wParam);
	//OutputDebugString(str);

	return S_OK;
}