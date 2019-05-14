// @Author Senwang
// @Email senwang94@gmail.com

#pragma once
#include "Connection.h"
#include "Buffer.h"
#include "base/noncopy.h"
#include <string>
#include <functional>
#include <unordered_map>
#include <iostream>
using namespace std;


#define HTTP_VERSION_9                 9
#define HTTP_VERSION_10                1000
#define HTTP_VERSION_11                1001
#define HTTP_VERSION_20                2000


#define HTTP_UNKNOWN                   0x0001
#define HTTP_GET                       0x0002
#define HTTP_HEAD                      0x0004
#define HTTP_POST                      0x0008
#define HTTP_PUT                       0x0010
#define HTTP_DELETE                    0x0020
#define HTTP_MKCOL                     0x0040
#define HTTP_COPY                      0x0080
#define HTTP_MOVE                      0x0100
#define HTTP_OPTIONS                   0x0200
#define HTTP_PROPFIND                  0x0400
#define HTTP_PROPPATCH                 0x0800
#define HTTP_LOCK                      0x1000
#define HTTP_UNLOCK                    0x2000
#define HTTP_PATCH                     0x4000
#define HTTP_TRACE                     0x8000



typedef struct 
{
	unordered_map<string, string> head_in;

}http_head_in_t;

typedef struct
{
	unordered_map<string, string>head_out;

}http_head_out_t;

typedef struct
{
	string body;
}http_request_body_t;

typedef struct
{
	
	http_head_in_t headers_in;
	http_head_out_t headers_out;
	http_request_body_t request_body;

	int method;
	int http_version;
	string request_line;
	string uri;
	string args;
	string exten;
	string unparsed_uri;

	char*    uri_start;
	char*    uri_end;
	char*    uri_ext;
	char*    args_start;
	char*    request_start;
	char*    request_end;
	char*    method_end;
	char*    schema_start;
	char*    schema_end;
	char*    host_start;
	char*    host_end;
	char*    port_start;
	char*    port_end;


	char*    header_name_start;
	char*    header_name_end;
	char*    header_start;
	char*    header_end;

	int invalid_header;


	int state;
	int sub_state;

	unsigned                          complex_uri : 1;
	unsigned                          quoted_uri : 1;
	unsigned                          plus_in_uri : 1;
	unsigned                          space_in_uri : 1;

	unsigned                          http_minor : 16;
	unsigned                          http_major : 16;

}http_request_t;


class httpRequest:noncopy
{
public:
	httpRequest():r(new http_request_t())
	{ 
		assert(r);
		r->state = 1; 
		r->sub_state = 1;
	}
	int http_handle_read(Buffer*);
	int http_handle_write(Buffer*,int);

private:

	int http_parse_request_line(Buffer*);
	int http_parse_header_line(Buffer*);
	int http_read_request_body(Buffer*);

	http_request_t* r;

};
