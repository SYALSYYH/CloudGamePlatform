#pragma once

#include "CefBrowserEventHandler.h"
// CUserGameDlg �Ի���

class CUserGameDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CUserGameDlg)

public:
	CUserGameDlg(const CString strServIP, const int nReslution, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CUserGameDlg();

// �Ի�������
	enum { IDD = IDD_DLG_USERGAME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL	OnInitDialog();

	void			InitBrower();
	afx_msg LRESULT OnCreateNewPage(WPARAM wParam, LPARAM lParam);//WM_CREATE_NEW_PAGE
	afx_msg LRESULT OnStartRemoteGame(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCloseRemoteGame(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();

	int		GetGameType();
	void	sendRemoteCloseMsg();

private:
	HWND								m_hWndBrowser;
	CefRefPtr<CCefBrowserEventHandler>  m_objBrowserEvent;
	int									m_nGameType;
	CString							    m_strRemoteIP;
	int									m_nReslution;
	CRemoteDesktopClientDlg*			m_pDlgRemote;

	int							        m_bRemoteRunStatus;//1 run 0 close
};
