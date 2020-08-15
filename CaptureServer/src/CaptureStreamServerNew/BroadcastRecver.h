#pragma once

typedef void(*OnBroadcastEvent)(int nEventType);//0:��ѯ����ռ��״̬ 1:��ѯ��������״̬

class CBroadcastRecver
{
public:

	static CBroadcastRecver& getInstance();
	
	~CBroadcastRecver();

	int startBroadcastRecv(OnBroadcastEvent cbEvent);
	void setMatchineInfo(int nStatus, int nType = 0);//nStatus:0���� 1ռ��
	int stopBroadcastRecv();

	OnBroadcastEvent m_cbBroadcastEvent;
	bool m_bExitThread;
	int  m_nStatus;
	int  m_nMacType;

protected:
	static CBroadcastRecver* s_pBcRecv;
	CBroadcastRecver();

};

