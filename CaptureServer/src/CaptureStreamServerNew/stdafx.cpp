// stdafx.cpp : ֻ������׼�����ļ���Դ�ļ�
// CaptureStreamServer.pch ����ΪԤ����ͷ
// stdafx.obj ������Ԥ����������Ϣ

#include "stdafx.h"

// TODO:  �� STDAFX.H ��
// �����κ�����ĸ���ͷ�ļ����������ڴ��ļ�������
string GetExePath()
{
	string strCurPath;
	char szBuffer[1024];
	memset(szBuffer, 0x00, sizeof(szBuffer));
	DWORD dwSize = GetModuleFileName(NULL, szBuffer, 1024);
	szBuffer[dwSize] = 0;
	while (szBuffer[dwSize] != '\\' && dwSize != 0)
	{
		szBuffer[dwSize] = 0; dwSize--;
	}
	//ȥ�����һ��"\\"
	if (0 <= dwSize)
	{
		szBuffer[dwSize] = 0;
	}
	strCurPath = szBuffer;
	//�滻���е�"\\"
	string::size_type   pos(0);
	string strNewValue = "/";
	string strOldValue = "\\";
	while ((pos = strCurPath.find(strOldValue.c_str(), 0)) != string::npos)
	{
		strCurPath.replace(pos, strOldValue.length(), strNewValue.c_str());
	}
	return strCurPath;
}