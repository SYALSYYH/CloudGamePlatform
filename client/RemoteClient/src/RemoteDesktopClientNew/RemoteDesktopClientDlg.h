
// RemoteDesktopClientDlg.h : ͷ�ļ�
//

#pragma once
#include "NetConnect.h"
#include "resource.h"
#include "afxwin.h"
#include <map>
using std::map;

// CRemoteDesktopClientDlg �Ի���
class CRemoteDesktopClientDlg : public CDialogEx
{
// ����
public:
	CRemoteDesktopClientDlg(const CString strServIP,const int nResolution,const int nGameType,CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_REMOTEDESKTOPCLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	BOOL InitAll();
	void ReleaseAll();
	void PlayRemoteDesktop();
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnClose();
	afx_msg LRESULT OnInput(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMouseLeave();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnKybordEvent(WPARAM wParam, LPARAM lParam);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);

	void SetMouseModel();

	HBITMAP MakeBitmap(HDC hDc, LPBYTE lpBits, long lWidth, long lHeight, WORD wBitCount);
	HCURSOR CreateCursorFromBitmap(HBITMAP hSourceBitmap, COLORREF clrTransparent, DWORD xHotspot,DWORD yHotspot);

	void SetRunTimeStatus(int nStatus){ m_nRunTimeStatus = nStatus; };
	int  GetRunTimeStatus(){ return m_nRunTimeStatus; }
	bool ConnectServer();

	int  PlayScreen();
	void setWindowsMode(int nMode);//0:����  2 ȫ��
	int GetStartProcessID(){ return m_nGameType; };

	bool Getkeystate(int nCode);
	bool AllkeysIsUp();
	void Setkystate(int nCode,bool bStatus);

private:
	
	CString      m_strServIP;
	CRect        m_OldRect;
	DWORD		 m_dwStyle;
	DWORD		 m_dwExStyle;
	int			 m_nDlgSizeType;

	int			 m_nEventStart;
	HCURSOR		 m_hHandCursor;
	
	int			 m_nRunTimeStatus;
	CStatic		 m_staPlayer;

	bool         m_bNetRet;

public:
	CString createMsgBody(int mMsg, int mValue1, int mValue2, long lParam);
	int		sendMsg(CString strMsg);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	int				m_nLastX;
	int				m_nLastY;

	int				m_nBaseX;
	int				m_nBaseY;
	CNetConnect		m_cNet;
	bool			m_bMouseRevel;
	bool            m_bMouseLeave;

	int				m_nResolution;//�ֱ��� 0:720p  1:1080P 2:�Զ�����������ķֱ���
	int				m_nWidth;
	int				m_nHeight;
	
	int				m_nGameType;//��������

	bool			m_bExitThread;
	HCURSOR			m_hCursor;

	HWND			m_hWndPlayer;

	HBITMAP			m_hBmpback;

	map<int, bool>   m_mapKeyStatus;

	bool			m_bF11Enable;//�Ƿ��Ѿ�������F11
	bool			m_bMouseDown;//����Ƿ�δƥ����л���Ϣ��
};
