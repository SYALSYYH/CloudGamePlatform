// LoginDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "RemoteDesktopClient.h"
#include "LoginDlg.h"
#include "afxdialogex.h"
#include "Util.h"

// CLoginDlg �Ի���

IMPLEMENT_DYNAMIC(CLoginDlg, CDialogEx)

CLoginDlg::CLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLoginDlg::IDD, pParent)
	, m_strRemoteIP("")
	, m_nRadioSel(0)
{

}

CLoginDlg::~CLoginDlg()
{
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_720, m_nRadioSel);
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CLoginDlg::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CLoginDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	string strExePath = GetExeFileExistDir() + "/Server.ini";
	char *strConfigFilePath = (char*)strExePath.c_str();
	char szGuid[50];
	memset(szGuid, 0, 50);
	GetPrivateProfileString("info", "ip", "", szGuid, 50, strConfigFilePath);
	GetDlgItem(IDC_EDIT_IP)->SetWindowText(szGuid);//192.168.1.101 

	return TRUE;
}
// CLoginDlg ��Ϣ�������

CString CLoginDlg::getRemoteIP()
{
	return m_strRemoteIP;
}
int CLoginDlg::getRemoteResolution()
{
	return m_nRadioSel;
}

void CLoginDlg::OnBnClickedOk()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	GetDlgItem(IDC_EDIT_IP)->GetWindowText(m_strRemoteIP);
	string strExePath = GetExeFileExistDir() + "/Server.ini";
	char *strConfigFilePath = (char*)strExePath.c_str();
	WritePrivateProfileString("info", "ip", m_strRemoteIP, strConfigFilePath);
	CDialogEx::OnOK();
}
