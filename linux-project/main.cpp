// @Author Wang sen
// @Email senwang94@gmail.com

#include "Server.h"
#include "EventLoop.h"
#include "base/Logging.h"
#include <string>
using namespace std;

int main()
{
	string IP("192.168.172.132");
	string Port("5000");
	string name("Webserver");
	string logpath = "./Webserver.log";
	Logger::setLogFileName(logpath);
	EventLoop loop;

	Server server(&loop, IP, Port, name, 0);
	server.start();
	LOG << " Webserver start !";
	loop.loop();
}