#include "TcpSocket.h"
#include "Socket.h"
#include "log.h"

using namespace xop;

TcpSocket::TcpSocket(SOCKET sockfd)
	: _sockfd(sockfd)
{
	
}

TcpSocket::~TcpSocket()
{
	
}

bool TcpSocket::bind(std::string ip, uint16_t port)
{
	struct sockaddr_in addr = {0};			  
    addr.sin_family = AF_INET;		  
    addr.sin_addr.s_addr = inet_addr(ip.c_str()); 
    addr.sin_port = htons(port);  
	
	if(::bind(_sockfd, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		LOG(" <socket=%d> bind <%s:%u> failed.\n", _sockfd, ip.c_str(), port);
		return false;
	}
	
	return true;
}

bool TcpSocket::listen(int backlog)
{
	if(::listen(_sockfd, backlog) == SOCKET_ERROR)
	{
		LOG("<socket=%d> listen failed.\n", _sockfd);
		return false;
	}
	
	return true;
}


SOCKET TcpSocket::accept()
{
	struct sockaddr_in addr = {0};
	socklen_t addrlen = sizeof addr;
	
	SOCKET clientfd = ::accept(_sockfd, (struct sockaddr*)&addr, &addrlen);
	
	return clientfd;
}

bool TcpSocket::connect(std::string ip, uint16_t port)
{
	struct sockaddr_in addr = {0};  		
	socklen_t addrlen = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip.c_str());

	if(::connect(_sockfd, (struct sockaddr*)&addr, addrlen) == SOCKET_ERROR)
	{
		LOG("<socket=%d> connect failed.\n", _sockfd);
		return false;
	}
	
	return true;
}

void TcpSocket::close()
{
#if defined(__linux) || defined(__linux__) 
	::close(_sockfd);
#elif defined(WIN32) || defined(_WIN32)
	closesocket(_sockfd);
#else
	
#endif
}

void TcpSocket::shutdownWrite()
{
	shutdown(_sockfd, SHUT_WR);
}
