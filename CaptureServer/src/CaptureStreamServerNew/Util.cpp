#include "stdafx.h"
#include "Util.h"
#include <objbase.h>  
#include <time.h>
#include <stdio.h>

unsigned int IPStrToInt(const char *ip)
{
	unsigned uResult = 0;
	int nShift = 24;
	int temp = 0;
	const char *pStart = ip;
	const char *pEnd = ip;

	while (*pEnd != '\0')
	{
		while (*pEnd != '.' && *pEnd != '\0')
		{
			pEnd++;
		}
		temp = 0;
		for (pStart; pStart != pEnd; ++pStart)
		{
			temp = temp * 10 + *pStart - '0';
		}

		uResult += temp << nShift;
		nShift -= 8;

		if (*pEnd == '\0')
			break;
		pStart = pEnd + 1;
		pEnd++;
	}

	return uResult;
}
string strCurPath = "";
string GetExeFileExistDir()
{
	if (!strCurPath.empty())
	{
		return strCurPath;
	}
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
  
string CreateGuidString(bool bFlag) //Ϊ�˷�ֹ�ھ��Զ̵�ʱ������������GUID���������ʱ������������С�ظ�����
{
	string strGuid;
	char buffer[40];
	GUID guid;
	time_t ltime;
	time(&ltime);

	if (CoCreateGuid(&guid))
	{
		fprintf(stderr, "create guid error\n");
	}
	else
	{
		guid.Data1 = (DWORD)ltime; //�����ʱ���
		guid.Data2 = (WORD)rand(); //��������
		guid.Data3 = (WORD)rand(); //��������

		if (bFlag)
		{
			_snprintf(buffer, sizeof(buffer),
				"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
				guid.Data1, guid.Data2, guid.Data3,
				guid.Data4[0], guid.Data4[1], guid.Data4[2],
				guid.Data4[3], guid.Data4[4], guid.Data4[5],
				guid.Data4[6], guid.Data4[7]);
		}
		else
		{
			_snprintf(buffer, sizeof(buffer),
				"%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
				guid.Data1, guid.Data2, guid.Data3,
				guid.Data4[0], guid.Data4[1], guid.Data4[2],
				guid.Data4[3], guid.Data4[4], guid.Data4[5],
				guid.Data4[6], guid.Data4[7]);
		}

		strGuid = buffer;

	}
	return strGuid;

}

//EWX_LOGOFF ע�� EWX_REBOOT ����  EWX_SHUTDOWN �ػ�
BOOL ReSetWindows(DWORD dwFlags, BOOL bForce)
{
	//�������Ƿ���ȷ  
	if (dwFlags != EWX_LOGOFF&&dwFlags != EWX_REBOOT&&dwFlags != EWX_SHUTDOWN)
		return FALSE;
	//���ϵͳ�İ汾��Ϣ�������Ǻ���ȷ���Ƿ���Ҫ����ϵͳȨ��  
	OSVERSIONINFO osvi = { 0 };
	//��ò����Ĵ�С�����ṩ��GetVersionEx���ж�����һ���°汾��OSVERSIONINFO�����Ǿɰ汾��  
	//�°汾��OSVERSIONINFOEX������汾  
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!GetVersionEx(&osvi))
	{
		return FALSE;
	}
	//������ϵͳ�İ汾�������NT���͵�ϵͳ����Ҫ����ϵͳȨ��  
	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		//EnableShutDownPriv();  
	}
	//�ж��Ƿ���ǿ�ƹػ���ǿ�ƹر�ϵͳ���̡�  
	dwFlags |= (bForce != FALSE) ? EWX_FORCE : EWX_FORCEIFHUNG;
	//����API  
	return ExitWindowsEx(dwFlags, 0);
}


unsigned char ToHex(unsigned char x)
{
	return  x > 9 ? x + 55 : x + 48;
}

unsigned char FromHex(unsigned char x)
{
	unsigned char y;
	if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
	else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
	else if (x >= '0' && x <= '9') y = x - '0';
	else assert(0);
	return y;
}

string UrlEncode(string str)
{
	string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		if (isalnum((unsigned char)str[i]) ||
			(str[i] == '-') ||
			(str[i] == '_') ||
			(str[i] == '.') ||
			(str[i] == '~'))
			strTemp += str[i];
		else if (str[i] == ' ')
			strTemp += "+";
		else
		{
			strTemp += '%';
			strTemp += ToHex((unsigned char)str[i] >> 4);
			strTemp += ToHex((unsigned char)str[i] % 16);
		}
	}
	return strTemp;
}

string UrlDecode(string str)
{
	string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		if (str[i] == '+') strTemp += ' ';
		else if (str[i] == '%')
		{
			assert(i + 2 < length);
			unsigned char high = FromHex((unsigned char)str[++i]);
			unsigned char low = FromHex((unsigned char)str[++i]);
			strTemp += high * 16 + low;

		}
		else strTemp += str[i];
	}
	return strTemp;
}