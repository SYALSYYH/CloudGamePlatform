#include "stdafx.h"
#include "HttpUtil.h"
#include <WS2tcpip.h>

CHttpUtil::CHttpUtil()
{
}


CHttpUtil::~CHttpUtil()
{
}

int CHttpUtil::getVmInfo(char* szIP, int nPort, int nNetType, CString strAreaId, CString& strResult)
{
	int nRet = -1;
	//��ʼ����socket��ʼ��;
	WSADATA wData;
	::WSAStartup(MAKEWORD(2, 2), &wData);

	SOCKET clientSocket = socket(AF_INET, 1, 0);
	struct sockaddr_in ServerAddr = { 0 };
	int Ret = 0;
	int AddrLen = 0;
	HANDLE hThread = 0;

	char bufSend[500];
	sprintf_s(bufSend,"GET /vm/consume/%d/%s HTTP/1.1\r\n"
		"Host:%s\r\n"
		"Connection:Keep-Alive\r\n"
		"Accept: */*\r\n"
		"Accept-Language:zh-CN,en,*\r\n"
		"User-Agent:Mozilla/5.0\r\n\r\n", nNetType, strAreaId, szIP);

	ServerAddr.sin_family = AF_INET;
	inet_pton(AF_INET, szIP, (void*)&ServerAddr.sin_addr);
	ServerAddr.sin_port = htons(nPort);

	char bufRecv[1024] = { 0 };
	int errNo = 0;
	errNo = connect(clientSocket, (sockaddr*)&ServerAddr, sizeof(ServerAddr));
	if (errNo == 0)
	{
		//������ͳɹ����򷵻ط��ͳɹ����ֽ���;
		if (send(clientSocket, bufSend, strlen(bufSend), 0)>0)
		{
			//������ܳɹ����򷵻ؽ��ܵ��ֽ���;
			if (recv(clientSocket, bufRecv, 1024, 0)>0)
			{
				strResult = CString(bufRecv);
				nRet = 0;
			}
		}
	}
	else
	{
		nRet = WSAGetLastError();
	}
	//socket��������;
	::WSACleanup();
	return nRet;
}

int CHttpUtil::getVmInfo(char* szIP, int nPort, CString strAreaId, CString& strResult)
{
	int nRet = -1;
	//��ʼ����socket��ʼ��;
	WSADATA wData;
	::WSAStartup(MAKEWORD(2, 2), &wData);

	SOCKET clientSocket = socket(AF_INET, 1, 0);
	struct sockaddr_in ServerAddr = { 0 };
	int Ret = 0;
	int AddrLen = 0;
	HANDLE hThread = 0;

	char bufSend[500];
	sprintf_s(bufSend, "GET /vm/consume/%s HTTP/1.1\r\n"
		"Host:%s\r\n"
		"Connection:Keep-Alive\r\n"
		"Accept: */*\r\n"
		"Accept-Language:zh-CN,en,*\r\n"
		"User-Agent:Mozilla/5.0\r\n\r\n", strAreaId, szIP);

	ServerAddr.sin_family = AF_INET;
	inet_pton(AF_INET, szIP, (void*)&ServerAddr.sin_addr);
	ServerAddr.sin_port = htons(nPort);

	char bufRecv[1024] = { 0 };
	int errNo = 0;
	errNo = connect(clientSocket, (sockaddr*)&ServerAddr, sizeof(ServerAddr));
	if (errNo == 0)
	{
		//������ͳɹ����򷵻ط��ͳɹ����ֽ���;
		if (send(clientSocket, bufSend, strlen(bufSend), 0)>0)
		{
			//������ܳɹ����򷵻ؽ��ܵ��ֽ���;
			if (recv(clientSocket, bufRecv, 1024, 0)>0)
			{
				strResult = CString(bufRecv);
				nRet = 0;
			}
		}
	}
	else
	{
		nRet = WSAGetLastError();
	}
	//socket��������;
	::WSACleanup();
	return nRet;
}
