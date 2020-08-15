// UserGameDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "RemoteDesktopClient.h"
#include "UserGameDlg.h"
#include "afxdialogex.h"


// CUserGameDlg �Ի���

IMPLEMENT_DYNAMIC(CUserGameDlg, CDialogEx)

CUserGameDlg::CUserGameDlg(const CString strServIP, const int nReslution, CWnd* pParent /*=NULL*/)
: CDialogEx(CUserGameDlg::IDD, pParent)
, m_hWndBrowser(NULL)
, m_nGameType(1)
, m_strRemoteIP(strServIP)
, m_nReslution(nReslution)
, m_pDlgRemote(NULL)
, m_bRemoteRunStatus(0)
{

}

CUserGameDlg::~CUserGameDlg()
{
}

void CUserGameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CUserGameDlg, CDialogEx)
	ON_MESSAGE(WM_CREATE_NEW_PAGE, OnCreateNewPage)
	ON_MESSAGE(WM_START_REMOTE, OnStartRemoteGame)
	ON_MESSAGE(WM_CLOSE_REMOTE, OnCloseRemoteGame)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CUserGameDlg ��Ϣ�������


BOOL CUserGameDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
//	ShowWindow(SW_MAXIMIZE);

	InitBrower();
	//if (m_hWndBrowser != NULL)
	//{
	//	m_objBrowserEvent->CloseBrowser(m_hWndBrowser);
	//}
	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣:  OCX ����ҳӦ���� FALSE
}

void CUserGameDlg::InitBrower()
{
	CefWindowInfo cefWindowInfo;
	//cefWindowInfo.SetAsChild(m_PaintManager.GetPaintWindow(), rtPositon);
	std::string strUrl = "http://www.sanjieyou.com/game/index.html";//"http://127.0.0.1:8080/html/em/customPluginDemo.html"
	CefString str;
	str.FromString(strUrl);
	CefBrowserSettings browserSetting;
	m_objBrowserEvent = CefRefPtr<CCefBrowserEventHandler>(new CCefBrowserEventHandler(this));
	CefBrowserHost::CreateBrowser(cefWindowInfo, m_objBrowserEvent, str, browserSetting, NULL);
}

LRESULT CUserGameDlg::OnCreateNewPage(WPARAM wParam, LPARAM lParam)
{
	m_hWndBrowser = (HWND)wParam;

	CRect rc;
	GetClientRect(&rc);
//	rc.bottom -= 40;
	::MoveWindow(m_hWndBrowser, rc.left, rc.top, rc.Width(), rc.Height(), TRUE);
	::ShowWindow(m_hWndBrowser, SW_SHOW | SW_SHOWNOACTIVATE);
	return 0;
}

LRESULT CUserGameDlg::OnStartRemoteGame(WPARAM wParam, LPARAM lParam)
{
	m_nGameType = (int)wParam;
	//CString str;
	//str.Format("start game:%d.", m_nGameType);
//	MessageBox(str);
	//if (m_hWndBrowser)
	//{
	//	m_objBrowserEvent->CloseBrowser(m_hWndBrowser);
	//}
	m_objBrowserEvent->GetBrowser(m_hWndBrowser)->GetMainFrame()->LoadURL("http://www.sanjieyou.com/game/index.html");
	if (m_bRemoteRunStatus == 1)
	{
		MessageBox("����Ϸ���������У����Ƚ������ٵ��!");
		return 0;
	}
	if (m_pDlgRemote != NULL)
	{
		delete m_pDlgRemote;
		m_pDlgRemote = NULL;
	}
	
	ShowWindow(SW_MINIMIZE);

	//CRemoteDesktopClientDlg dlg(m_strRemoteIP, m_nReslution, m_nGameType);
	//
	//CWndsManger::getWndsMangerInstance().setMainDlg(&dlg);
	//INT_PTR nResponse = dlg.DoModal();
	m_pDlgRemote = new CRemoteDesktopClientDlg(m_strRemoteIP, m_nReslution, m_nGameType);
	m_pDlgRemote->Create(IDD_REMOTEDESKTOPCLIENT_DIALOG, this);
	m_pDlgRemote->ShowWindow(SW_SHOW);
	CWndsManger::getWndsMangerInstance().setMainDlg(m_pDlgRemote);
	m_pDlgRemote->SetFocus();
	m_bRemoteRunStatus = 1;
//	ShowWindow(SW_SHOWNORMAL);
	//CefShutdown();
	//CDialog::OnOK();
	return 0;
}

LRESULT CUserGameDlg::OnCloseRemoteGame(WPARAM wParam, LPARAM lParam)
{
	sendRemoteCloseMsg();
	return 0;
}

int CUserGameDlg::GetGameType()
{
	return m_nGameType;
}
//
//void CUserGameDlg::OnBnClickedButton1()
//{
//	// TODO:  �ڴ���ӿؼ�֪ͨ����������
//	m_objBrowserEvent->GetBrowser(m_hWndBrowser)->GetMainFrame()->LoadURL("www.baidu.com");
//}


void CUserGameDlg::OnClose()
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CefShutdown();
	CDialogEx::OnClose();
}

void CUserGameDlg::sendRemoteCloseMsg()
{
	ShowWindow(SW_SHOWNORMAL);
	m_bRemoteRunStatus = 0;
}