// @Author Wang sen
// @Email senwang94@gmail.com

#pragma once
#include "base/noncopy.h"
#include "base/Logging.h"
#include "Connection.h"
#include "EventLoop.h"
#include "Channel.h"
#include "EventLoopThreadPool.h"
#include "Socket.h"
#include <string>
#include <memory>
#include <map>

class Server :noncopy
{
	enum Option
	{
		kNoReusePort,
		kReusePort,
	};
public:
	Server(EventLoop* loop, const std::string& IP, const std::string& Port,
		const std::string& nameArg, int threadNums_);
	~Server();

	void start();
private:
	typedef std::shared_ptr<Connection> ConnectionPtr;
	typedef std::map<std::string, ConnectionPtr> ConnectionMap;

	void handleConn();

	void removeConnection(const ConnectionPtr& conn);
	/// Not thread safe, but in loop
	void removeConnectionInLoop(const ConnectionPtr& conn);


	int listenfd_;
	int nextConnId_;

	EventLoop* loop_;
	const std::string IP_;
	const std::string Port_;
	std::string name_;
	std::shared_ptr<EventLoopThreadPool> threadPool_;
	std::shared_ptr<Channel> ListenChannel_;
	std::unique_ptr<Sockets>socketfd_;
	bool started_;
	int threadNums_;

	ConnectionMap connections_;
};
