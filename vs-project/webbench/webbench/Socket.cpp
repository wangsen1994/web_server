// @Author Senwang
// @Email senwang94@gmail.com

#include "Socket.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/uio.h>
#include <signal.h>
#include <string.h>
#include <netinet/tcp.h>
#include <iostream>




ssize_t Sockets::read(void *buf, size_t count)
{
	return ::read(sockfd_, buf, count);
}
ssize_t Sockets::readv(const struct iovec *iov, int iovcnt)
{
	return ::readv(sockfd_, iov, iovcnt);
}
ssize_t Sockets::write(const void *buf, size_t count)
{
	return ::write(sockfd_, buf, count);
}

void Sockets::close()
{
	if (::close(sockfd_) < 0)
	{
		// std::cout << "socket close error " << std::endl;
	}
}
void Sockets::shutdownWrite()
{
	if (::shutdown(sockfd_, SHUT_WR) < 0)
	{
		// std::cout << "sockets::shutdownWrite " << std::endl;
	}
}

int Sockets::setSocketNonBlocking()
{
	int flag = fcntl(sockfd_, F_GETFL, 0);
	if (flag == -1)
		return -1;

	flag |= O_NONBLOCK;
	if (fcntl(sockfd_, F_SETFL, flag) == -1)
		return -1;
	return 0;
}
void Sockets::setSocketNodelay()
{
	int enable = 1;
	setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
}
void Sockets::setSocketNoLinger()
{
	struct linger linger_;
	linger_.l_onoff = 1;
	linger_.l_linger = 30;
	setsockopt(sockfd_, SOL_SOCKET, SO_LINGER, (const char *)&linger_, sizeof(linger_));
}