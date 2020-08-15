#pragma once
#include "afxwin.h"


// CMsgTips �Ի���

class CMsgTips : public CDialogEx
{
	DECLARE_DYNAMIC(CMsgTips)

public:
	static CMsgTips& getMsgTipsInstance();
	virtual ~CMsgTips();

	virtual BOOL OnInitDialog();
// �Ի�������
	enum { IDD = IDD_DLG_TIPS };


	void SetTipText(CString strTips);

	void CloseTipsDlg();

protected:
	CMsgTips(CWnd* pParent = NULL);   // ��׼���캯��
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

	static CMsgTips*  s_pMsgTipsDlg;
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
private:
	CEdit m_editTips;
	CFont m_newFont;
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
