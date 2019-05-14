// @Author Senwang
// @Email senwang94@gmail.com

#pragma once
#include <arpa/inet.h>

class Sockets
{
public:
	explicit Sockets(int sockfd)
		: sockfd_(sockfd)
	{ }

	ssize_t read(void *buf, size_t count);
	ssize_t readv(const struct iovec *iov, int iovcnt);
	ssize_t write(const void *buf, size_t count);
	void close();
	void shutdownWrite();

	int setSocketNonBlocking();
	void setSocketNodelay();
	void setSocketNoLinger();
private:
	const int sockfd_;
};
