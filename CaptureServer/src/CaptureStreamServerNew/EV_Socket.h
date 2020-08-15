

#ifndef _EV_SOCKET_H
#define _EV_SOCKET_H
# include <string>
using namespace std;


#define BASIC_RECEIVE_SICHUANG		(1024*1024*2)

class CDayeBasicSocket
{
public:

	// ����TCP socket�� ����-1ʧ��
	static int socket();

	// �ر�socket
	static void close(int socket);

	// ����socketΪ������ʽ ����ʱ����falseʧ��
	static bool setNonBlocking(int socket);

	// ��ָ����socket�϶�ȡ���� ����falseʧ��
	static bool nbRead(int socket, std::string& s, bool *eof, int& nError,int nTimeout = 20 );

	// ��ָ����socket�϶�ȡָ�����ȵ����� ����falseʧ��
	static bool nbRead(int socket, std::string& s, int nReadLen,bool *eof, int& nError);

	// д�����ݵ�ָ����socket�� ����falseʧ��
	//static bool nbWrite(int socket, std::string& s, int *bytesSoFar);
	static bool nbWrite(int socket, char *pSendBuffer, int nSendSize, int *bytesSoFar );

	// ���º���ʹ���ڷ�����

	// ������������
	static bool setReuseAddr(int socket);

	// ���������ر�
	static bool SetNoLinger(int socket);

	// ����tcp_nodelay
	static bool SetTcpNodelay(int socket);

	// �󶨵�ָ���˿�
	static bool bind(int socket, int port);

	// �ڶ˿��ϼ���
	static bool listen(int socket, int backlog);

	// ��������
	static int accept(int socket);

	// ���º��������ڿͻ���

	// ���ӵ�������
	static bool connect(int socket, std::string& host, int port);

	// �������һ������
	static int getError();

	// ��������������
	static std::string getErrorMsg();

#if defined(WIN32)
	// ���ݴ���ŷ�������
	static std::string getErrorMsg(int error);
#endif

public:
	// ���ݽ���buf
	static char m_pcRev[BASIC_RECEIVE_SICHUANG];

};
#endif