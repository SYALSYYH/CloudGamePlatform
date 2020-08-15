
// RemoteDesktopClient.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "RemoteDesktopClient.h"
#include "RemoteDesktopClientDlg.h"
#include "LoginDlg.h"
#include "HttpUtil.h"
#include "CefBrowserApp.h"
#include "Util.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//
//#include<string>
//using std::string;
// CRemoteDesktopClientApp


bool CheckProccessRunInstance()
{
	HANDLE hProcessHandle = ::CreateMutex(NULL, FALSE, "Global\\REMOTEDESKTOPCLIENT");

	int nLastError = GetLastError();
	if (ERROR_ALREADY_EXISTS == nLastError || ERROR_ACCESS_DENIED == nLastError)
	{
		//		MessageBox("Applictaion is running !");
		return false;
	}

	return true;
}

BEGIN_MESSAGE_MAP(CRemoteDesktopClientApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CRemoteDesktopClientApp ����

CRemoteDesktopClientApp::CRemoteDesktopClientApp()
{
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
	// TODO:  �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CRemoteDesktopClientApp ����

CRemoteDesktopClientApp theApp;


// CRemoteDesktopClientApp ��ʼ��

BOOL CRemoteDesktopClientApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()��  ���򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// ���� shell ���������Է��Ի������
	// �κ� shell ����ͼ�ؼ��� shell �б���ͼ�ؼ���
	CShellManager *pShellManager = new CShellManager;

	// ���Windows Native���Ӿ����������Ա��� MFC �ؼ�����������
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO:  Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));


	bool bOnly = CheckProccessRunInstance();
	if (!bOnly)
	{
		AfxMessageBox("�Ѿ������ͻ��ˣ��޷���������!");
		return TRUE;
	}

//	freopen(".\\log.txt", "w", stderr);
	//CString str = (CString)AfxGetApp()->m_lpCmdLine;
	//AfxMessageBox(str);

	//�����ɰ汾��huqb
	// CEF Init
	//CefEnableHighDPISupport();
	//CefSettings settings;
	//settings.no_sandbox = true;
	//settings.multi_threaded_message_loop = true;
	//settings.single_process = true;
	//CefRefPtr<CCefBrowserApp> objApp(new CCefBrowserApp());
	//CefMainArgs mainArgs(AfxGetInstanceHandle());
	//CefInitialize(mainArgs, settings, objApp.get() /*NULL*/, NULL);


	CString strRemoteIP = "";
	int     nResolution = 0;
	int nGameType = 1;

//���԰汾
	CLoginDlg dlgLogin;
	INT_PTR nRes = dlgLogin.DoModal();
	if (nRes == IDOK)
	{
		strRemoteIP = dlgLogin.getRemoteIP();
		nResolution = dlgLogin.getRemoteResolution();
	}
	else
	{
		CefShutdown();
		return FALSE;
	}//���԰汾

	/*
	HTTP/1.1 200
	Content-Type: application/json;charset=UTF-8
	Transfer-Encoding: chunked
	Date: Wed, 27 Jun 2018 04:43:37 GMT

	c3
	{"id":"5B321375002948238F8375047284BE72","rmIp":"","ip":"192.168.1.14","port":"554","description":"","net":0,"runState":1,"areaId":2,"lastStateTime":1530074599709,"lastConsumeTime":1530074617530}
	0
	*/

	//�ֶ�����IP�İ汾��ֱ�ӻ�ȡIP b
	/*string strExePath = GetExeFileExistDir() + "/Server.ini";
	char *strConfigFilePath = (char*)strExePath.c_str();
	char szIP[50];
	memset(szIP, 0, 50);
	GetPrivateProfileString("info", "ip", NULL, szIP, 50, strConfigFilePath);
	strRemoteIP = szIP;
	if (strRemoteIP.IsEmpty())
	{
		AfxMessageBox("�����쳣���������Ա��ϵ!");
		return TRUE;
	}*/
	//�ֶ�����IP�İ汾��ֱ�ӻ�ȡIP e

	//�Զ���ȡIP b
	//string strExePath = GetExeFileExistDir() + "/Server.ini";
	//char *strConfigFilePath = (char*)strExePath.c_str();
	//char szGuid[50];
	//memset(szGuid, 0, 50);
	//GetPrivateProfileString("info", "id", NULL, szGuid, 50, strConfigFilePath);

	//CString strRes;
	//CHttpUtil cGetUtil;
	//CString strAreaID = szGuid;
	////int nRet = cGetUtil.getVmInfo("116.62.205.17", 80, 2, strAreaID, strRes);
	//int nRet = cGetUtil.getVmInfo("116.62.205.17", 80, strAreaID, strRes);
	//if (nRet != 0)
	//{
	//	AfxMessageBox("���������Ƿ�ͨ������Զ�̷����Ƿ�����");
	//	return FALSE;
	//}
	//
	////�򵥽���
	//int nPos = strRes.Find("\"ip\":\"");//Find("\"ip\":\"");
	//if (nPos > 0)
	//{
	//	CString strIP = strRes.Right(strRes.GetLength() - nPos - 6);
	//	nPos = strIP.Find("\",");
	//	if (nPos > 0)
	//	{
	//		strRemoteIP = strIP.Left(nPos);
	//	}
	//	else
	//	{
	//		AfxMessageBox("����������������ϵ����Ա��");
	//	}
	//}
	//else
	//{
	//	AfxMessageBox("�����п��л��������Ժ����Ի���ϵ����Ա��");
	//	return FALSE;
	//}
	//�Զ���ȡIP e
	
	//�����ɰ汾��huqb
	//CUserGameDlg dlgGame(strRemoteIP, nResolution);
	//m_pMainWnd = &dlgGame;
	//CWndsManger::getWndsMangerInstance().setUserGameDlg(&dlgGame);
	//INT_PTR nResponse = dlgGame.DoModal();

	CRemoteDesktopClientDlg dlg(strRemoteIP, nResolution, nGameType);
	m_pMainWnd = &dlg;
	CWndsManger::getWndsMangerInstance().setMainDlg(&dlg);
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO:  �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO:  �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "����: �Ի��򴴽�ʧ�ܣ�Ӧ�ó���������ֹ��\n");
		TRACE(traceAppMsg, 0, "����: ������ڶԻ�����ʹ�� MFC �ؼ������޷� #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS��\n");
	}
//	CefShutdown();

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

