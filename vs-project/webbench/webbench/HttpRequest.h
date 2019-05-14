// @Author Senwang
// @Email senwang94@gmail.com

#pragma once
#include "Buffer.h"
#include "noncopy.h"
#include <string>
#include <map>
#include <iostream>

using namespace std;

class HttpRequest :noncopy
{

public:
	struct Request_line
	{
		string method;
		string path;
		string Version;
		string files;    // for GET 
	};
	enum HttpRequestParseState
	{
		kExpectRequestLine,
		kExpectHeaders,
		kExpectBody,
		kGotAll,
	};


public:
	HttpRequest(Buffer* buffer);
	~HttpRequest();

	void parseRequest();
	void CreateResponse(Buffer* buffer);

private:
	Buffer* buffer_;
	HttpRequestParseState state_;
	Request_line request_line_;
	map<string, string> header_line;
	string body;
};

