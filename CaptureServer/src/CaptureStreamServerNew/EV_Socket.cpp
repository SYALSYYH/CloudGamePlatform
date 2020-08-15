#include "stdafx.h"
#include "Ev_Socket.h"

#if defined(_WINDOWS) || WIN32
# include <stdio.h>
# include <winsock2.h>
#include <time.h>
#include <string>
#include <errno.h>

using std::string;

# define EINPROGRESS	WSAEINPROGRESS
# define EWOULDBLOCK	WSAEWOULDBLOCK
# define ETIMEDOUT	    WSAETIMEDOUT

#pragma comment( lib, "ws2_32.lib" )

static void initWinSock()
{
	static bool wsInit = false;
	if (! wsInit)
	{
		WORD wVersionRequested = MAKEWORD( 2, 0 );
		WSADATA wsaData;
		WSAStartup(wVersionRequested, &wsaData);
		wsInit = true;
	}
}


#else

extern "C" {
# include <unistd.h>
# include <stdio.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <netdb.h>
# include <errno.h>
# include <fcntl.h>
# include <stdlib.h>
# include <signal.h>
#include <string.h>
}

static void initWinSock()
{
	static bool wsInit = false;
	if (! wsInit)
	{
		// ����ϵͳ�ڶԶ�socket close����µĴ���
		signal(SIGPIPE,SIG_IGN);
		wsInit = true;
	}
}

#endif  // _WINDOWS

static inline bool nonFatalError()
{
	int err = CDayeBasicSocket::getError();
	return (err == EINPROGRESS || err == EAGAIN || err == EWOULDBLOCK || err == EINTR || err == 0);
}

int CDayeBasicSocket::socket()
{
	initWinSock();
#if defined(WIN32)
	SOCKET sock = ::socket(AF_INET, SOCK_STREAM, 0);
#else
	int sock = ::socket(AF_INET, SOCK_STREAM, 0);
#endif
	if (sock >= 0)
	{
		int nrcvbuf = 1024*1024*4;
		setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&nrcvbuf, sizeof(nrcvbuf));
		int nsndbuf = 1024*1024;//524288;	//512KB
		setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&nsndbuf, sizeof(nsndbuf));
	}

	return (int) sock;
}

void CDayeBasicSocket::close(int fd)
{
#if defined(_WINDOWS) || WIN32
	closesocket(fd);
#else
	shutdown(fd, 2);
	::close(fd);
#endif // _WINDOWS
}

bool CDayeBasicSocket::setNonBlocking(int fd)
{
#if defined(_WINDOWS) || WIN32
	unsigned long flag = 1;
	return (ioctlsocket((SOCKET)fd, FIONBIO, &flag) == 1); //huqb
#else
	return (fcntl(fd, F_SETFL, O_NONBLOCK) == 0);
#endif // _WINDOWS
}

bool CDayeBasicSocket::setReuseAddr(int fd)
{
	// ����socket������
	int sflag = 1;
	return (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&sflag, sizeof(sflag)) == 0);
}

// ���������ر�
bool CDayeBasicSocket::SetNoLinger(int fd)
{
	linger lge;
	lge.l_onoff = 1;
	lge.l_linger = 0;
	return (setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char *)&lge, sizeof(lge)) == 0);
}

// ����tcp_nodelay
bool CDayeBasicSocket::SetTcpNodelay(int socket)
{
	int flag = 1;
	return (setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int)) == 0); 
}

// �󶨵�ָ���˿�
bool CDayeBasicSocket::bind(int fd, int port)
{
	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = htons((u_short) port);
	return (::bind(fd, (struct sockaddr *)&saddr, sizeof(saddr)) == 0);
}

// ����socketΪ����ģʽ
bool  CDayeBasicSocket::listen(int fd, int backlog)
{
	return (::listen(fd, backlog) == 0);
}

int CDayeBasicSocket::accept(int fd)
{
	struct sockaddr_in addr;
#if defined(_WINDOWS) || WIN32
	int
#else
	socklen_t
#endif

		addrlen = sizeof(addr);

	return (int) ::accept(fd, (struct sockaddr*)&addr, &addrlen);
}

// ���ӷ�����
bool CDayeBasicSocket::connect(int fd, std::string& host, int port)
{
	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;

	struct hostent *hp = gethostbyname(host.c_str());
	if (hp == 0) return false;

	saddr.sin_family = hp->h_addrtype;
	memcpy(&saddr.sin_addr, hp->h_addr, hp->h_length);
	saddr.sin_port = htons((u_short) port);

	// For asynch operation, this will return EWOULDBLOCK (windows) or
	// EINPROGRESS (linux) and we just need to wait for the socket to be writable...

	clock_t timeBegin = clock();
	int result = ::connect(fd, (struct sockaddr *)&saddr, sizeof(saddr));
	clock_t timeEnd = clock();

	if (result == -1)
	{
#if defined(WIN32)
		int nError = WSAGetLastError();		
#else
		int nError = errno;		
#endif

	}

	return result == 0 || nonFatalError();
}

bool CDayeBasicSocket::nbRead(int fd, std::string& s, bool *eof, int& nError, int nTimeout )
{
	static int nLastfd = -1;
	static int nLastError = 0;

	const int READ_SIZE = 4096;
	char readBuf[READ_SIZE] = {0};

	bool wouldBlock = false;
	*eof = false;

	struct timeval timeout;
	timeout.tv_sec = nTimeout;
	timeout.tv_usec = 0;

	fd_set outFd,inFd,excFd;
	FD_ZERO(&inFd);
	FD_ZERO(&outFd);
	FD_ZERO(&excFd);

	FD_SET(fd, &inFd);
	int nEvents = select(fd+1, &inFd, &outFd, &excFd, &timeout);
	if (nEvents<0)
	{
		return false;
	}
	if (nEvents == 0)
	{
		return false;
	}

	while ( ! wouldBlock && ! *eof)
	{
		int n = recv(fd, readBuf, READ_SIZE-1, 0);

		if (n > 0)
		{
			readBuf[n] = 0;
			s.append(readBuf, n);
		}
		else if (n == 0)
		{
			*eof = true;
		}
		else if (nonFatalError())
		{
			wouldBlock = true;
		}
		else
		{
#if defined(WIN32)
			nError = WSAGetLastError();
#else
			nError = errno;
#endif
			return false;
		}
	}
	return true;
}

// ��ָ����socket�϶�ȡָ�����ȵ����� ����falseʧ��
bool nbRead(int fd, std::string& s, int nReadLen,bool *eof, int& nError)
{
	static int nLastfd = -1;
	static int nLastError = 0;

	const int READ_SIZE = 4096;
	char readBuf[READ_SIZE];
	int nTotalRecvLen = 0;

	bool wouldBlock = false;
	*eof = false;

	while ( ! wouldBlock && ! *eof)
	{
		int nCurRecvLen = nReadLen - nTotalRecvLen;
		if (nCurRecvLen > READ_SIZE)
		{
			nCurRecvLen = READ_SIZE;
		}
		int n = recv(fd, readBuf, nCurRecvLen-1, 0);

		if (n > 0)
		{
			readBuf[n] = 0;
			s.append(readBuf, n);

			nTotalRecvLen+= n;
			if (nTotalRecvLen == nReadLen)
			{
				*eof = true;
			}
		}
		else if (nonFatalError())
		{
			wouldBlock = true;
		}
		else
		{
#if defined(WIN32)
			nError = WSAGetLastError();
#else
			nError = errno;
#endif
			return false;
		}
	}
	return true;
}

// д������
bool CDayeBasicSocket::nbWrite(int fd, char *pSendBuffer, int nSendSize, int *bytesSoFar)
{
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	static int nLastfd = -1;
	static int nLastError = 0;
	
	if ( NULL == pSendBuffer || nSendSize <= 0 )
	{
		return false;
	}
	int nToWrite = nSendSize;//int(s.length()) - *bytesSoFar;
	char *sp = const_cast<char*>(pSendBuffer) + *bytesSoFar;//const_cast<char*>(s.c_str()) + *bytesSoFar;
	bool wouldBlock = false;
	int sendsize = 0;
	char *writep = sp;

	fd_set outFd,inFd,excFd;
	FD_ZERO(&inFd);
	FD_ZERO(&outFd);
	FD_ZERO(&excFd);

	FD_SET(fd, &outFd);
	int nEvents = select(fd+1, &inFd, &outFd, &excFd, &timeout);
	if (nEvents<0)
	{

		return false;
	}
	if (nEvents == 0)
	{
		return false;
	}

	while ( nToWrite > 0 && ! wouldBlock )
	{
		int nCurrentSendSize = nToWrite;
		if (nCurrentSendSize > 64*1024)
		{
			nCurrentSendSize = 64*1024;
		}
		int n = send(fd, sp, nCurrentSendSize, 0);

		if (n > 0)
		{
			sp += n;
			*bytesSoFar += n;
			nToWrite -= n;
			sendsize += n;
		}
		else if (nonFatalError())
		{
			wouldBlock = true;

			// �ڷ���buf����ʱ,�����δ����������ȴ�յ���������������,��Ҫ��������߼�������
			if( sendsize != nSendSize )
			{
//				printf("|||| Ҫ�������� %d,  ʵ�ʷ��ͳ���:  %d \r\n.", nSendSize, sendsize );
				return true;	
			}
		}
		else
		{
#if defined(WIN32)
			int nError = WSAGetLastError();
#else
			int nError = errno;
#endif
			if ((nError != nLastError) && (nLastfd != fd))
			{
				nLastfd = fd;
				nLastError = nError;
			}
			else
			{
			}

			return false;   // Error
		}
	}

	if ( nSendSize == sendsize )
	{
//		printf("Ҫ�������� %d,  ʵ�ʷ��ͳ���:  %d.\r\n", nSendSize, sendsize );
	}
	else
	{
		printf("�������ݴ���......Ҫ�������� %d,  ʵ�ʷ��ͳ���:  %d.\r\n", nSendSize, sendsize );
	}
	
	return true;
}

// �������һ������
int CDayeBasicSocket::getError()
{
#if defined(_WINDOWS)
	return WSAGetLastError();
#else
	return errno;
#endif
}

// ��������������
std::string CDayeBasicSocket::getErrorMsg()
{
#if defined(WIN32)
	return getErrorMsg(getError());
#else
	return string(strerror(errno));
#endif
}

// ���ݴ���ŷ�������
std::string CDayeBasicSocket::getErrorMsg(int nError)
{
	switch (nError)
	{
	case WSAEWOULDBLOCK:
		return "�޷��������һ�����赲���׽��ֲ���";
		break;
	case WSAEINPROGRESS:
		return "Ŀǰ����ִ��һ���赲�Բ���";
		break;
	case WSAEALREADY:
		return "��һ�����赲�׽����ϳ�����һ���Ѿ��ڽ��еĲ���";
		break;
	case WSAENOTSOCK:
		return "��һ�����׽����ϳ�����һ������";
		break;
	case WSAEDESTADDRREQ:
		return "����ĵ�ַ��һ���׽����дӲ����к���";
		break;
	case WSAEMSGSIZE:
		return "һ�������ݱ��׽����Ϸ��͵���Ϣ�����ڲ���Ϣ������������һЩ�������ƣ�����û����ڽ������ݱ��Ļ����������ݱ�С";
		break;
	case WSAEPROTOTYPE:
		return "���׽��ֺ���������ָ����һ��Э�鲻֧��������׽������͵��﷨";
		break;
	case WSAENOPROTOOPT:
		return "�� getsockopt �� setsockopt ������ָ����һ��δ֪�ġ���Ч�Ļ���֧�ֵ�ѡ�����";
		break;
	case WSAEPROTONOSUPPORT:
		return "�����Э�黹û����ϵͳ�����ã�����û�������ڵļ���";
		break;
	case WSAESOCKTNOSUPPORT:
		return "�������ַ�����в����ڶ�ָ���Ĳ�����͵�֧��";
		break;
	case WSAEOPNOTSUPP:
		return "�ο��Ķ������Ͳ�֧�ֳ��ԵĲ���";
		break;
	case WSAEPFNOSUPPORT:
		return "Э�������δ���õ�ϵͳ�л�û�����Ĵ��ڼ���";
		break;
	case WSAEAFNOSUPPORT:
		return "ʹ�����������Э�鲻���ݵĵ�ַ";
		break;
	case WSAEADDRINUSE:
		return "ͨ��ÿ���׽��ֵ�ַ(Э��/�����ַ/�˿�)ֻ����ʹ��һ��";
		break;
	case WSAEADDRNOTAVAIL:
		return "�����������У�������ĵ�ַ��Ч";
		break;
	case WSAENETDOWN:
		return "�׽��ֲ���������һ������������";
		break;
	case WSAENETUNREACH:
		return "��һ���޷����ӵ����糢����һ���׽��ֲ���";
		break;
	case WSAENETRESET:
		return "���ò����ڽ����У����ڱ��ֻ�Ĳ�����⵽һ�����ϣ��������ж���";
		break;
	case WSAECONNABORTED:
		return "���������е����������һ���ѽ���������";
		break;
	case WSAECONNRESET:
		return "Զ������ǿ�ȹر���һ�����е�����";
		break;
	case WSAENOBUFS:
		return "����ϵͳ�������ռ䲻����ж�����������ִ���׽����ϵĲ���";
		break;
	case WSAEISCONN:
		return "��һ���Ѿ����ӵ��׽���������һ����������";
		break;
	case WSAENOTCONN:
		return "�����׽���û�����Ӳ���(��ʹ��һ�� sendto ���÷������ݱ��׽���ʱ)û���ṩ��ַ�����ͻ�������ݵ�����û�б�����";
		break;
	case WSAESHUTDOWN:
		return "������ǰ�Ĺرյ��ã��׽������Ǹ������Ѿ��رգ����ͻ�������ݵ�����û�б�����";
		break;
	case WSAETOOMANYREFS:
		return "��ĳ���ں˶�������ù���";
		break;
	case WSAETIMEDOUT:
		return "�������ӷ���һ��ʱ���û����ȷ�𸴻����ӵ�����û�з�Ӧ�����ӳ���ʧ��";
		break;
	case WSAECONNREFUSED:
		return "����Ŀ����������ܾ����޷�����";
		break;
	case WSAELOOP:
		return "�޷�ת������";
		break;
	case WSAENAMETOOLONG:
		return "�������������̫��";
		break;
	case WSAEHOSTDOWN:
		return "����Ŀ���������ˣ��׽��ֲ���ʧ��";
		break;
	case WSAEHOSTUNREACH:
		return "�׽��ֲ�������һ���޷����ӵ�����";
		break;
	case WSAENOTEMPTY:
		return "����ɾ��Ŀ¼���������ǿյ�";
		break;
	case WSAEPROCLIM:
		return "һ�� Windows �׽��ֲ��������ڿ���ͬʱʹ�õ�Ӧ�ó�����Ŀ��������";
		break;
	case WSAEUSERS:
		return "�޶��";
		break;
	case WSAEDQUOT:
		return "�����޶��";
		break;
	case WSAESTALE:
		return "�ļ�������ò��ٿ���";
		break;
	case WSAEREMOTE:
		return "��Ŀ�ڱ��ز�����";
		break;

	case WSASYSNOTREADY:
		return "��Ϊ��ʹ���ṩ��������ϵͳĿǰ��Ч��WSAStartup Ŀǰ������������";
		break;
	case WSAVERNOTSUPPORTED:
		return "��֧������� Windows �׽��ְ汾";
		break;
	case WSANOTINITIALISED:
		return "Ӧ�ó���û�е��� WSAStartup������ WSAStartup ʧ��";
		break;
	case WSAEDISCON:
		return "�� WSARecv �� WSARecvFrom ���ر�ʾԶ�̷����Ѿ���ʼ�˹رղ���";
		break;
	case WSAENOMORE:
		return "WSALookupServiceNext ���ܷ��ظ���Ľ��";
		break;
	case WSAECANCELLED:
		return "�ڸõ��û����ڴ�����ʱ���͵����� WSALookupServiceEnd���õ��ñ�ȡ��";
		break;
	case WSAEINVALIDPROCTABLE:
		return "���̵�����Ч";
		break;
	case WSAEINVALIDPROVIDER:
		return "����ķ����ṩ������Ч";
		break;
	case WSAEPROVIDERFAILEDINIT:
		return "�޷����ػ��ʼ������ķ����ṩ����";
		break;
	case WSASYSCALLFAILURE:
		return "������Ӧʧ�ܵ�ϵͳ����ʧ����";
		break;
	case WSASERVICE_NOT_FOUND:
		return "�˷��񲻴��ڡ���ָ�������ƿռ����Ҳ��������";
		break;
	case WSATYPE_NOT_FOUND:
		return "�Ҳ���ָ�������";
		break;
	case WSA_E_NO_MORE:
		return "WSALookupServiceNext ���ܷ��ظ���Ľ��";
		break;
	case WSA_E_CANCELLED:
		return "�ڸõ��û����ڴ�����ʱ���͵����� WSALookupServiceEnd���õ��ñ�ȡ��";
		break;
	case WSAEREFUSED:
		return "���ڱ��ܾ������ݲ�ѯʧ��";
		break;
	default:
		return "δ֪����";
		break;
	}
	return "unknown socket error";
}