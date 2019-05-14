// @Author Senwang
// @Email senwang94@gmail.com

#include "Connection.h"
#include "EventLoop.h"
#include "Socket.h"

#include "Channel.h"
#include <memory>
#include <iostream>

class EventLoop;
class Sockets;
class Channel;


Connection::Connection(EventLoop* loop,
	const std::string& Con_name,
	int sockfd)
	:loop_(loop),
	state_(kConnecting),
	Con_name_(Con_name),
	socket_(new Sockets(sockfd)),
	reading_(true),
	channel_(new Channel(loop, sockfd)),
	highWaterMark_(64 * 1024 * 1024),
	req(new httpRequest())
{
	channel_->setReadCallback(
		std::bind(&Connection::handleRead, this));
	channel_->setWriteCallback(
		std::bind(&Connection::handleWrite, this));
	channel_->setCloseCallback(
		std::bind(&Connection::handleClose, this));
	channel_->setErrorCallback(
		std::bind(&Connection::handleError, this));
	// socket_->setKeepAlive(true);
}

Connection::~Connection()
{
	// std::cout << "  ~Connection  fd " << channel_->fd() << endl;
	assert(state_ == kDisconnected);
}

void Connection::connectEstablished()
{
	loop_->assertInLoopThread();
	assert(state_ == kConnecting);
	setState(kConnected);
	channel_->tie(shared_from_this());
	channel_->enableReading();
	ConnectionPtr guardThis(shared_from_this());
	// loop_->runTimer(guardThis);  // 关闭定时器
}


void Connection::startRead()
{
	loop_->runInLoop(std::bind(&Connection::startReadInLoop, this));
}

void Connection::startReadInLoop()
{
	loop_->assertInLoopThread();
	assert(channel_);
	if (!reading_ || !channel_->isReading())
	{
		channel_->enableReading();
		reading_ = true;
	}
}
void Connection::send()
{
	if (state_ == kConnected)
	{
		loop_->assertInLoopThread();
		ssize_t  nwrote = 0;
		ssize_t remaining = outputBuffer_.readableBytes();
		if (!channel_->isWriting())
		{
			nwrote = socket_->write(outputBuffer_.peek(), outputBuffer_.readableBytes());
			if (nwrote >= 0)
			{
				remaining -= nwrote;
				outputBuffer_.retrieve(nwrote);
			}
			else
			{
				nwrote = 0;
				if (errno != EWOULDBLOCK)
				{
					// cout << "TcpConnection::sendInLoop" << endl;
					if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
					{
						// cout << "TcpConnection::send Error" << endl;
					}
				}
			}
			if (remaining > 0 && !channel_->isWriting())
			{
				channel_->enableWriting();
			}
			else if (remaining == 0)
			{
				shutdownInLoop();
			}
		}
	}
}

void Connection::shutdown()
{
	// FIXME: use compare and swap
	if (state_ == kConnected)
	{
		setState(kDisconnecting);
		// FIXME: shared_from_this()?
		loop_->runInLoop(std::bind(&Connection::shutdownInLoop, this));
	}
}

void Connection::shutdownInLoop()  // 写完时主动关闭
{
	loop_->assertInLoopThread();
	if (!channel_->isWriting())
	{
		// we are not writing
		setState(kDisconnected);
		channel_->disableAll();
		ConnectionPtr guardThis(shared_from_this());
		closeCallback_(guardThis);
		// std::cout << "-----shutdownInloop------" << endl;
	}
}

void Connection::handleRead()
{
	loop_->assertInLoopThread();
	int savedErrno = 0;
	ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
	int ret;
	if (n > 0)
	{
		//*************处理 buffer***************// 
		// 这里还需要考虑粘包与合并包的问题，可以利用buffer的prepend预留部分来记录每个包的长度
		// 解析包时再来判断是否为一个合格的包，如果不合格则丢弃。

		// 对于包的解析可以使用protocBuf来做序列化与反序列化

		// cout << "----before parse----" << inputBuffer_.readableBytes() << endl;
		// std::cout << " read: " << n << std::endl;

		// HttpRequest request(&inputBuffer_);
		// inputBuffer_.retrieveAll();
		// request.CreateResponse(&outputBuffer_);
		// send();
		ret = req->http_handle_read(&inputBuffer_);
		if (ret == HTTP_ERROR||ret==HTTP_OK)
		{
			req->http_handle_write(&outputBuffer_,ret);
			if(inputBuffer_.readableBytes()==0)
				send();
		}

		// request.parseRequest();
		// cout << "----after parse----" << inputBuffer_.readableBytes() << endl;

		// forceClose();
		// loop_->queueInLoop(std::bind(&Connection::shutdown, this));
	}
	else if (n == 0)
	{
		handleClose();
	}
	else
	{
		errno = savedErrno;
		handleError();
	}
}
void Connection::handleWrite()
{
	loop_->assertInLoopThread();
	if (channel_->isWriting())
	{
		ssize_t n = socket_->write(outputBuffer_.peek(), outputBuffer_.readableBytes());
		if (n > 0)
		{
			outputBuffer_.retrieve(n);
			if (outputBuffer_.readableBytes() == 0)
			{
				channel_->disableWriting();
				shutdownInLoop();              /* 写完后关闭 */
			}
		}
		else
		{
			// std::cout << "TcpConnection::handleWrite" << std::endl;
		}
	}
	else
	{
		// std::cout << "Connection fd = " << channel_->fd()
		// 	<< " is down, no more writing" << std::endl;
	}
}

void Connection::handleError()
{
	/* 出错后强制关闭 */
	forceClose();
}

void Connection::forceClose()
{
	// FIXME: use compare and swap
	if (state_ == kConnected || state_ == kDisconnecting)
	{
		setState(kDisconnecting);
		loop_->queueInLoop(std::bind(&Connection::forceCloseInLoop, shared_from_this()));
	}
}
void Connection::forceCloseInLoop()
{
	loop_->assertInLoopThread();
	if (state_ == kConnected || state_ == kDisconnecting)
	{
		// as if we received 0 byte in handleRead();
		handleClose();
	}
}
void Connection::connectDestroyed()   // 在server中删除掉后回调
{
	loop_->assertInLoopThread();
	if (state_ == kConnected)
	{
		setState(kDisconnected);
		channel_->disableAll();   
	}
	channel_->remove(); // 本连接对应的描述符不在需要监控，从poll中删除。
	socket_->close();
}
void Connection::handleClose()
{
	// std::cout << "----handleclose----" << endl;
	loop_->assertInLoopThread();
	assert(state_ == kConnected || state_ == kDisconnecting);   

	setState(kDisconnected);
	channel_->disableAll();
	ConnectionPtr guardThis(shared_from_this());
	closeCallback_(guardThis);
}

