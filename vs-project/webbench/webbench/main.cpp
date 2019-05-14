// @Author Wang sen
// @Email senwang94@gmail.com

#include "Server.h"
#include "EventLoop.h"
#include <string>
using namespace std;

int main()
{
	string IP("192.168.172.132");
	string Port("5000");
	string name("Webserver");
	EventLoop loop;

	Server server(&loop, IP, Port, name, 4);
	server.start();
	loop.loop();
}