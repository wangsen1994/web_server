// @Author Senwang
// @Email senwang94@gmail.com

#pragma once
#include "noncopy.h"
#include "Buffer.h"
#include "EventLoop.h"
#include "HttpRequest.h"
#include "Timer.h"

#include <memory>
#include <string>

class EventLoop;
class Sockets;
class Channel;

class Connection : noncopy, public std::enable_shared_from_this<Connection>
{
	typedef std::shared_ptr<Connection> ConnectionPtr;
	typedef std::function<void(const ConnectionPtr&)> CloseCallback;

public:
	Connection(EventLoop* loop,
		const std::string& Con_name,
		int sockfd
	);
	~Connection();


	bool connected() const { return state_ == kConnected; }
	bool disconnected() const { return state_ == kDisconnected; }

	/*
	void send(Buffer* message);  // this one will swap data
	void sendInLoop(const void* message, size_t len);
	*/
	void send();  // this one will swap data

	void shutdown(); // NOT thread safe, no simultaneous calling

	void startRead();

	bool isReading() const { return reading_; };

	void connectEstablished();
	EventLoop* getLoop() const { return loop_; }
	const string& name() const { return Con_name_; }

	void setCloseCallback(const CloseCallback& cb)
	{
		closeCallback_ = cb;
	}
	void connectDestroyed();

	int getFd()
	{
		return sockfd_;
	}
private:

	enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
	void handleRead();
	void handleWrite();
	void handleClose();
	void handleError();

	void startReadInLoop();
	void shutdownInLoop();

	void parseRequest(Buffer);

	void forceClose();
	void forceCloseInLoop();
	void setState(StateE s) { state_ = s; }


	int sockfd_;
	EventLoop* loop_;
	const std::string Con_name_;
	StateE state_;
	bool reading_;
	CloseCallback closeCallback_;
	std::unique_ptr<Sockets> socket_;
	std::unique_ptr<Channel> channel_;

	size_t highWaterMark_;
	Buffer inputBuffer_;
	Buffer outputBuffer_;
};

