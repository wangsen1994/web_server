// @Author Wang sen
// @Email senwang94@gmail.com

#include "Server.h"
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <functional>
#include <fcntl.h> 

int ini_listenfd(const std::string& IP, const std::string& Port)
{
	int port = stoi(Port);
	if (port < 0 || port > 65535)
	{
		// std::cout << "listen port error" << std::endl;
		return -1;
	}
	int listen_fd = 0;
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		// std::cout << "socket create error" << std::endl;
		return -1;
	}
	int optval = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
	{
		// std::cout << "socket set error" << std::endl;
		return -1;
	}

	// 设置服务器IP和Port，和监听描述副绑定
	struct sockaddr_in server_addr;
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP.c_str());
	server_addr.sin_port = htons((unsigned short)port);
	if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		// std::cout << "socket bind error" << std::endl;
		return -1;
	}

	// 开始监听，最大等待队列长为LISTENQ
	if (listen(listen_fd, 2048) == -1)
	{
		// std::cout << "socket listen error" << std::endl;
		return -1;
	}

	// 无效监听描述符
	if (listen_fd == -1)
	{
		close(listen_fd);
		return -1;
	}
	fcntl(listen_fd, F_SETFL, fcntl(listen_fd, F_GETFL, 0) | O_NONBLOCK);
	return listen_fd;
}

void handle_for_sigpipe()
{
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	if (sigaction(SIGPIPE, &sa, NULL))
		return;
}

Server::Server(EventLoop* loop, const std::string& IP, const std::string& Port,
	const std::string& nameArg, int threadNums)
	:loop_(loop),
	IP_(IP), Port_(Port), name_(nameArg), threadNums_(threadNums),
	threadPool_(new EventLoopThreadPool(loop, name_)),
	listenfd_(ini_listenfd(IP, Port)),
	socketfd_(new Sockets(listenfd_)),
	ListenChannel_(new Channel(loop, listenfd_)),
	started_(false),
	nextConnId_(1)
{
	std::cout << "listen fd :" << listenfd_ << std::endl;
	handle_for_sigpipe();
	socketfd_->setSocketNodelay();
}
Server::~Server() { }

void Server::start()
{
	threadPool_->setThreadNum(threadNums_);
	threadPool_->start();

	ListenChannel_->setReadCallback(std::bind(&Server::handleConn, this));
	ListenChannel_->enableReading();

	started_ = true;

}
void Server::handleConn()
{
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(struct sockaddr_in));
	socklen_t client_addr_len = sizeof(client_addr);
	int accept_fd = 0;

	while ((accept_fd = accept(listenfd_, (struct sockaddr*)&client_addr, &client_addr_len)) > 0)
	{

		EventLoop *loop = threadPool_->getNextLoop();
		 LOG << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port)
			<< " fd = " << accept_fd;


		if (accept_fd >= 10000) // 最大连接数量
		{
			close(accept_fd);
			continue;
		}
		// 设为非阻塞模式
		Sockets(accept_fd).setSocketNonBlocking();
		Sockets(accept_fd).setSocketNodelay();

		std::string con_name(std::string("socket_"));
		con_name += to_string(accept_fd);   // 这里不能用int类型 会出错
		ConnectionPtr con(new Connection(loop, con_name, accept_fd));
		connections_[con_name] = con;
		con->setCloseCallback(
			std::bind(&Server::removeConnection, this, std::placeholders::_1));
		loop->runInLoop(std::bind(&Connection::connectEstablished, con));
	}
}
void Server::removeConnection(const ConnectionPtr& conn)
{
	// FIXME: unsafe
	loop_->runInLoop(std::bind(&Server::removeConnectionInLoop, this, conn));
}

void Server::removeConnectionInLoop(const ConnectionPtr& conn)
{
	loop_->assertInLoopThread();

	// std::cout <<"removeConnectionInLoop"<< "conn: " << conn->name() << "  fd " << conn->getFd() << endl;
	size_t n = connections_.erase(conn->name());
	(void)n;
	assert(n == 1);
	EventLoop* ioLoop = conn->getLoop();
	// 注册的是 TcpConnection::connectDestroyed
	ioLoop->queueInLoop(
		std::bind(&Connection::connectDestroyed, conn));  //先清除主线程中channel再清除epoll中channel
}