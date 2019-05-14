// @Author Senwang
// @Email senwang94@gmail.com

#include "HttpRequest.h"
#include "Buffer.h"


HttpRequest::HttpRequest(Buffer* buffer)
	:state_(kExpectRequestLine),
	buffer_(buffer)
{
}

HttpRequest::~HttpRequest() {}

void HttpRequest::parseRequest()
{
	const char* pos1 = buffer_->findCRLF();
	string URI = string(buffer_->peek(), pos1);
	buffer_->retrieve(pos1 - buffer_->peek() + 2);

	if (state_ == kExpectRequestLine)
	{
		int pos = URI.find(' ');
		request_line_.method = string(URI.begin(), URI.begin() + pos);
		URI = URI.substr(pos + 1);
		pos = URI.find('/');
		URI = URI.substr(pos + 1);
		if ((pos = URI.find('?')) != -1)
		{
			request_line_.path = string(URI.begin(), URI.begin() + pos);
			URI = URI.substr(pos + 1);
			pos = URI.find(' ');
			request_line_.files = string(URI.begin(), URI.begin() + pos);
			URI = URI.substr(pos + 1);
		}
		else
		{
			pos = URI.find(' ');
			request_line_.files = string(URI.begin(), URI.begin() + pos);
			URI = URI.substr(pos + 1);
		}
		request_line_.Version = URI;
		state_ = kExpectHeaders;
	}

	if (state_ == kExpectHeaders)
	{
		char* pos2 = const_cast<char*>(buffer_->findCRLF());
		string line = string(buffer_->peek(), pos2 - buffer_->peek());
		while (!line.empty())
		{
			int tmp = line.find(':');
			string key = string(line.begin(), line.begin() + tmp);
			string value = line.substr(tmp + 2);
			header_line[key] = value;
			buffer_->retrieve(pos2 - buffer_->peek() + 2);
			pos2 = const_cast<char*>(buffer_->findCRLF());
			line = string(buffer_->peek(), pos2 - buffer_->peek());
		}
		state_ = kExpectBody;
	}
	if (state_ == kExpectBody)
	{
		buffer_->retrieve(2);
		body = string(buffer_->peek(), buffer_->readableBytes());
		buffer_->retrieve(body.size());
		state_ = kGotAll;
	}
	if (state_ == kGotAll)
	{
		/*
		std::cout << "------------request line-----------------" << std::endl;
		std::cout << request_line_.method << " " << request_line_.path << " " << request_line_.files << " " << request_line_.Version << std::endl;
		std::cout << "------------request header-----------------" << std::endl;
		for (auto itr = header_line.begin(); itr != header_line.end(); itr++)
		{
		  std::cout << itr->first << " : " << itr->second << std::endl;
		}
		std::cout << "------------request body-----------------" << std::endl;
		std::cout << body << std::endl;
		*/
	}
}

void HttpRequest::CreateResponse(Buffer* buffer)
{
	string short_msg = string("OK");
	short_msg = " " + short_msg;

	string body_buff, header_buff;
	body_buff += "hi senwang";
	// body_buff += "<html><title>hi senwang</title>";
	// body_buff += "<body bgcolor=\"ffffff\">";
	// body_buff += to_string(200) + short_msg;
	// body_buff += "<hr><em> SenWang's Web Server</em>\n</body></html>";

	header_buff += "HTTP/1.1 " + to_string(200) + short_msg + "\r\n";
	header_buff += "Content-type: text/html\r\n";
	//header_buff += "Connection: Keep-Alive\r\n";
	//header_buff += "Content-length: " + to_string(body_buff.size()) + "\r\n";
	header_buff += "\r\n";

	buffer->append(header_buff.c_str(), header_buff.size());
	buffer->append(body_buff.c_str(), body_buff.size());
}