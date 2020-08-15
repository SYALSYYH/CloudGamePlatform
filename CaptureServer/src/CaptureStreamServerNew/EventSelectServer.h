/////////////////////////////////////////////////////
// EventSelectServer.h�ļ�

int IoDomain(OnEventUserIOStatus pStatusCb);

//�������
typedef struct _NETHEARTBEATINFO
{
	void*		  pThread;
	//	PSOCKET_OBJ   pSocket;
	long		  lLastTime;//ms
	_NETHEARTBEATINFO()
	{
		pThread = NULL;
		//		pSocket = NULL;
		lLastTime = 0;
	}
}NETHEARTBEATINFO, *PNETHEARTBEATINFO;

// �׽��ֶ���
typedef struct _SOCKET_OBJ
{
	SOCKET s;					// �׽��־��
	HANDLE event;				// ����׽�����������¼�������
	sockaddr_in addrRemote;		// �ͻ��˵�ַ��Ϣ

	bool  bSendTag;				// �ܷ��ʹ����ݿ�ı�־
	bool  bBeginHeatbeat;       // ������������־
	PNETHEARTBEATINFO  pHeartInfo;//����

	_SOCKET_OBJ *pNext;			// ָ����һ��SOCKET_OBJ����Ϊ��������һ����
} SOCKET_OBJ, *PSOCKET_OBJ;

// �̶߳���
typedef struct _THREAD_OBJ
{
	HANDLE events[WSA_MAXIMUM_WAIT_EVENTS];	// ��¼��ǰ�߳�Ҫ�ȴ����¼�����ľ��
	int nSocketCount;						// ��¼��ǰ�̴߳�����׽��ֵ����� <=  WSA_MAXIMUM_WAIT_EVENTS

	PSOCKET_OBJ pSockHeader;				// ��ǰ�̴߳�����׽��ֶ����б�pSockHeaderָ���ͷ
	PSOCKET_OBJ pSockTail;					// pSockTailָ���β

	CRITICAL_SECTION cs;					// �ؼ�����α�����Ϊ����ͬ���Ա��ṹ�ķ���
	_THREAD_OBJ *pNext;						// ָ����һ��THREAD_OBJ����Ϊ��������һ����

} THREAD_OBJ, *PTHREAD_OBJ;



