
// RemoteDesktopClient.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CRemoteDesktopClientApp: 
// �йش����ʵ�֣������ RemoteDesktopClient.cpp
//

class CRemoteDesktopClientApp : public CWinApp
{
public:
	CRemoteDesktopClientApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CRemoteDesktopClientApp theApp;