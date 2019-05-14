// @Author Senwang
// @Email senwang94@gmail.com

#include "httphandler.h"
#include <unistd.h>

static uint32_t  usual[] = {
	0xffffdbfe, /* 1111 1111 1111 1111  1101 1011 1111 1110 */

				/* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
	0x7fff37d6, /* 0111 1111 1111 1111  0011 0111 1101 0110 */

				/* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
#if (NGX_WIN32)
	0xefffffff, /* 1110 1111 1111 1111  1111 1111 1111 1111 */
#else
	0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
#endif

				/*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
	0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

	0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
	0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
	0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
	0xffffffff  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
};


int httpRequest::http_parse_request_line(Buffer* buf)
{
	
	enum 
	{
		sw_start = 1,
		sw_method,
		sw_spaces_before_uri,
		sw_schema,
		sw_schema_slash,
		sw_schema_slash_slash,
		sw_host_start,
		sw_host,
		sw_host_end,
		sw_host_ip_literal,
		sw_port,
		sw_host_http_09,
		sw_after_slash_in_uri,
		sw_check_uri,
		sw_check_uri_http_09,
		sw_uri,
		sw_http_09,
		sw_http_H,
		sw_http_HT,
		sw_http_HTT,
		sw_http_HTTP,
		sw_first_major_digit,
		sw_major_digit,
		sw_first_minor_digit,
		sw_minor_digit,
		sw_spaces_after_digit,
		sw_almost_done
	};
	
	int state_ = r->sub_state;

	char c, ch, *p, *m;
	if (buf->get_pos() == NULL)
		p = const_cast<char*>(buf->peek());
	else
		p = buf->get_pos();

	for (; p < buf->beginWrite(); p++)
	{
		ch = *p;

		switch (state_)
		{
		case sw_start:

			r->request_start = p;
			if (ch == '\r' || ch == '\n')
			{
				break;
			}
			if ((ch<'A' || ch>'Z') && ch != '_')
			{
				return HTTP_ERROR;
			}

			state_ = sw_method;
			break;

		case sw_method:
			if (ch == ' ')
			{
				r->method_end = p - 1;
				m = r->request_start;

				string method = string(m, p - m);

				switch (p-m)
				{
				case 3:
					if (method == "GET") {
						r->method = HTTP_GET;
						break;
					}

					if (method == "PUT") {
						r->method = HTTP_PUT;
						break;
					}
					break;

				case 4:
					if (m[1] == 'O') {

						if (method == "POST") {
							r->method = HTTP_POST;
							break;
						}

						if (method == "COPY") {
							r->method = HTTP_COPY;
							break;
						}

						if (method == "MOVE") {
							r->method = HTTP_MOVE;
							break;
						}
						if (method == "LOCK") {
							r->method = HTTP_LOCK;
							break;
						}
					}
					else {

						if (method == "HEAD") {
							r->method = HTTP_HEAD;
							break;
						}
					}
					break;

				case 5:
					if (method == "PATCH") {
						r->method = HTTP_PATCH;
						break;
					}

					if (method == "TRACE") {
						r->method = HTTP_TRACE;
						break;
					}

					break;
				case 6:
					if (method == "DELETE") {
						r->method = HTTP_DELETE;
						break;
					}

					if (method == "UNLOCK") {
						r->method = HTTP_UNLOCK;
						break;
					}
					break;

				case 7:
					if (method == "OPTIONS")
					{
						r->method = HTTP_OPTIONS;
					}

					break;

				case 8:
					if (method == "PROPFIND")
					{
						r->method = HTTP_PROPFIND;
					}
					break;
				case 9:
					if (method == "PROPPATCH")
					{
						r->method = HTTP_PROPPATCH;
					}
					break;
				}

				state_ = sw_spaces_before_uri;
				break;
				}
				if ((ch < 'A' || ch > 'Z') && ch != '_') {
					return HTTP_ERROR;
				}
				break;


		case sw_spaces_before_uri:

			if (ch == '/') {
				r->uri_start = p;
				state_ = sw_after_slash_in_uri;
				break;
			}

			c = (u_char)(ch | 0x20);
			if (c >= 'a' && c <= 'z') {
				r->schema_start = p;
				state_ = sw_schema;
				break;
			}
			switch (ch) {
			case ' ':
				break;
			default:
				return HTTP_ERROR;
			}
			break;

		case sw_schema:

			c = (u_char)(ch | 0x20);
			if (c >= 'a' && c <= 'z') {
				break;
			}

			switch (ch) {
			case ':':
				r->schema_end = p;
				state_ = sw_schema_slash;
				break;
			default:
				return HTTP_ERROR;
			}
			break;

		case sw_schema_slash:
			switch (ch) {
			case '/':
				state_ = sw_schema_slash_slash;
				break;
			default:
				return HTTP_ERROR;
			}
			break;

		case sw_schema_slash_slash:
			switch (ch) {
			case '/':
				state_ = sw_host_start;
				break;
			default:
				return HTTP_ERROR;
			}
			break;

		case sw_host_start:

			r->host_start = p;

			if (ch == '[') {
				state_ = sw_host_ip_literal;
				break;
			}

			state_ = sw_host;

			/* fall through */

		case sw_host:

			c = (u_char)(ch | 0x20);
			if (c >= 'a' && c <= 'z') {
				break;
			}

			if ((ch >= '0' && ch <= '9') || ch == '.' || ch == '-') {
				break;
			}

			/* fall through */

		case sw_host_end:

			r->host_end = p;

			switch (ch) {
			case ':':
				state_ = sw_port;
				break;
			case '/':
				r->uri_start = p;
				state_ = sw_after_slash_in_uri;
				break;
			case ' ':

				r->uri_start = r->schema_end + 1;
				r->uri_end = r->schema_end + 2;
				state_ = sw_host_http_09;
				break;
			default:
				return HTTP_ERROR;
			}
			break;

		case sw_host_ip_literal:

			if (ch >= '0' && ch <= '9') {
				break;
			}

			c = (u_char)(ch | 0x20);
			if (c >= 'a' && c <= 'z') {
				break;
			}

			switch (ch) {
			case ':':
				break;
			case ']':
				state_ = sw_host_end;
				break;
			case '-':
			case '.':
			case '_':
			case '~':
				/* unreserved */
				break;
			case '!':
			case '$':
			case '&':
			case '\'':
			case '(':
			case ')':
			case '*':
			case '+':
			case ',':
			case ';':
			case '=':
				/* sub-delims */
				break;
			default:
				return HTTP_ERROR;
			}
			break;

		case sw_port:
			if (ch >= '0' && ch <= '9') {
				break;
			}

			switch (ch) {
			case '/':
				r->port_end = p;
				r->uri_start = p;
				state_ = sw_after_slash_in_uri;
				break;
			case ' ':
				r->port_end = p;
				/*
				 * use single "/" from request line to preserve pointers,
				 * if request line will be copied to large client buffer
				 */
				r->uri_start = r->schema_end + 1;
				r->uri_end = r->schema_end + 2;
				state_ = sw_host_http_09;
				break;
			default:
				return HTTP_ERROR;
			}
			break;

			/* space+ after "http://host[:port] " */
		case sw_host_http_09:
			switch (ch) {
			case ' ':
				break;
			case '\r':
				r->http_minor = 9;
				state_ = sw_almost_done;
				break;
			case '\n':
				r->http_minor = 9;
				goto done;
			case 'H':
				state_ = sw_http_H;
				break;
			default:
				return HTTP_ERROR;
			}
			break;


			/* check "/.", "//", "%", and "\" (Win32) in URI */
		case sw_after_slash_in_uri:

			if (usual[ch >> 5] & (1 << (ch & 0x1f))) {
				state_ = sw_check_uri;
				break;
			}

			switch (ch) {
			case ' ':
				r->uri_end = p;
				state_ = sw_check_uri_http_09;
				break;
			case '\r':
				r->uri_end = p;
				r->http_minor = 9;
				state_ = sw_almost_done;
				break;
			case '\n':
				r->uri_end = p;
				r->http_minor = 9;
				goto done;
			case '.':
				r->complex_uri = 1;
				state_ = sw_uri;
				break;
			case '%':
				r->quoted_uri = 1;
				state_ = sw_uri;
				break;
			case '/':
				r->complex_uri = 1;
				state_ = sw_uri;
				break;
			case '?':
				r->args_start = p + 1;
				state_ = sw_uri;
				break;
			case '#':
				r->complex_uri = 1;
				state_ = sw_uri;
				break;
			case '+':
				r->plus_in_uri = 1;
				break;
			case '\0':
				return HTTP_ERROR;
			default:
				state_ = sw_check_uri;
				break;
			}
			break;

			/* check "/", "%" and "\" (Win32) in URI */
		case sw_check_uri:

			if (usual[ch >> 5] & (1 << (ch & 0x1f))) {
				break;
			}

			switch (ch) {
			case '/':
				r->uri_ext = NULL;
				state_ = sw_after_slash_in_uri;
				break;
			case '.':
				r->uri_ext = p + 1;
				break;
			case ' ':
				r->uri_end = p;
				state_ = sw_check_uri_http_09;
				break;
			case '\r':
				r->uri_end = p;
				r->http_minor = 9;
				state_ = sw_almost_done;
				break;
			case '\n':
				r->uri_end = p;
				r->http_minor = 9;
				goto done;
			case '%':
				r->quoted_uri = 1;
				state_ = sw_uri;
				break;
			case '?':
				r->args_start = p + 1;
				state_ = sw_uri;
				break;
			case '#':
				r->complex_uri = 1;
				state_ = sw_uri;
				break;
			case '+':
				r->plus_in_uri = 1;
				break;
			case '\0':
				return HTTP_ERROR;
			}
			break;

			/* space+ after URI */
		case sw_check_uri_http_09:
			switch (ch) {
			case ' ':
				break;
			case '\r':
				r->http_minor = 9;
				state_ = sw_almost_done;
				break;
			case '\n':
				r->http_minor = 9;
				goto done;
			case 'H':
				state_ = sw_http_H;
				break;
			default:
				r->space_in_uri = 1;
				state_ = sw_check_uri;
				p--;
				break;
			}
			break;


			/* URI */
		case sw_uri:

			if (usual[ch >> 5] & (1 << (ch & 0x1f))) {
				break;
			}

			switch (ch) {
			case ' ':
				r->uri_end = p;
				state_ = sw_http_09;
				break;
			case '\r':
				r->uri_end = p;
				r->http_minor = 9;
				state_ = sw_almost_done;
				break;
			case '\n':
				r->uri_end = p;
				r->http_minor = 9;
				goto done;
			case '#':
				r->complex_uri = 1;
				break;
			case '\0':
				return HTTP_ERROR;
			}
			break;

			/* space+ after URI */
		case sw_http_09:
			switch (ch) {
			case ' ':
				break;
			case '\r':
				r->http_minor = 9;
				state_ = sw_almost_done;
				break;
			case '\n':
				r->http_minor = 9;
				goto done;
			case 'H':
				state_ = sw_http_H;
				break;
			default:
				r->space_in_uri = 1;
				state_ = sw_uri;
				p--;
				break;
			}
			break;

		case sw_http_H:
			switch (ch) {
			case 'T':
				state_ = sw_http_HT;
				break;
			default:
				return HTTP_ERROR;
			}
			break;

		case sw_http_HT:
			switch (ch) {
			case 'T':
				state_ = sw_http_HTT;
				break;
			default:
				return HTTP_ERROR;
			}
			break;

		case sw_http_HTT:
			switch (ch) {
			case 'P':
				state_ = sw_http_HTTP;
				break;
			default:
				return HTTP_ERROR;
			}
			break;

		case sw_http_HTTP:
			switch (ch) {
			case '/':
				state_ = sw_first_major_digit;
				break;
			default:
				return HTTP_ERROR;
			}
			break;

			/* first digit of major HTTP version */
		case sw_first_major_digit:
			if (ch < '1' || ch > '9') {
				return HTTP_ERROR;
			}

			r->http_major = ch - '0';
			state_ = sw_major_digit;
			break;

			/* major HTTP version or dot */
		case sw_major_digit:
			if (ch == '.') {
				state_ = sw_first_minor_digit;
				break;
			}

			if (ch < '0' || ch > '9') {
				return HTTP_ERROR;
			}

			r->http_major = r->http_major * 10 + ch - '0';
			break;

			/* first digit of minor HTTP version */
		case sw_first_minor_digit:
			if (ch < '0' || ch > '9') {
				return HTTP_ERROR;
			}

			r->http_minor = ch - '0';
			state_ = sw_minor_digit;
			break;

			/* minor HTTP version or end of request line */
		case sw_minor_digit:
			if (ch == '\r') {
				state_ = sw_almost_done;
				break;
			}

			if (ch == '\n') {
				goto done;
			}

			if (ch == ' ') {
				state_ = sw_spaces_after_digit;
				break;
			}

			if (ch < '0' || ch > '9') {
				return HTTP_ERROR;
			}

			r->http_minor = r->http_minor * 10 + ch - '0';
			break;

		case sw_spaces_after_digit:
			switch (ch) {
			case ' ':
				break;
			case '\r':
				state_ = sw_almost_done;
				break;
			case '\n':
				goto done;
			default:
				return HTTP_ERROR;
			}
			break;

			/* end of request line */
		case sw_almost_done:
			r->request_end = p - 1;
			switch (ch) {
			case '\n':
				goto done;
			default:
				return HTTP_ERROR;
			}
		}
	}

	buf->set_pos(p);
	// buf->retrieveUntil(p);
	r->sub_state= state_;
	return HTTP_AGAIN;
done:
	buf->set_pos(NULL);
	buf->retrieveUntil(p + 1);
	if (r->request_end == NULL)
	{
		r->request_end = p;
	}
	r->uri = string(r->uri_start, r->uri_end);
	r->request_line = string(r->request_start, r->request_end);
	r->http_version = r->http_major * 1000 + r->http_minor;
	r->sub_state = sw_start;
	return HTTP_OK;
}

int httpRequest::http_parse_header_line(Buffer* buf)
{
	char      c, ch, *p;

	enum {
		sw_start = 1,
		sw_name,
		sw_space_before_value,
		sw_value,
		sw_space_after_value,
		sw_almost_done,
		sw_header_almost_done,
		sw_done
	};

	int state_ = r->sub_state;

	if (buf->get_pos() == NULL)
		p = const_cast<char*>(buf->peek());
	else
		p = buf->get_pos();

	for (; p < buf->beginWrite(); p++)
	{
		ch = *p;

		switch (state_)
		{
		case sw_start:

			r->header_name_start = p;
			r->invalid_header = 0;
			switch (ch)
			{

			case '\r':
				r->header_end = p;
				state_ = sw_header_almost_done;
				break;

			case '\n':
				r->header_end = p;
				goto header_done;

			default:
				state_ = sw_name;

				c = char(ch | 0x20); /* 转为小写 */

				if (c >= 'a'&&c <= 'z'){ break; }
				if (ch == '-' || ch == '_' || ch == '~' || ch == '.') { break; }
				if (ch >= '0' && ch <= '9') { break; }

				if (ch == '\0') return HTTP_ERROR;
				break;

			}
			break;

		case sw_name:

			c = char(ch | 0x20);
			if (c >= 'a'&&c <= 'z') { break; }

			if (ch == ':')
			{
				r->header_name_end = p;
				state_ = sw_space_before_value;
				break;
			}
			if (ch == '-' || ch == '_' || ch == '~' || ch == '.') { break; }
			if (ch >= '0' && ch <= '9') { break; }

			if (ch == '\r')
			{
				r->header_name_end = p;
				r->header_start = p;
				r->header_end = p;
				state_ = sw_almost_done;
				break;
			}
			if (ch == '\n')
			{
				r->header_name_end = p;
				r->header_start = p;
				r->header_end = p;
				goto done;
			}
			if (ch == '\0') return HTTP_ERROR;

			break;

		case sw_space_before_value:
			switch (ch)
			{
			case ' ':
				break;
			case '\r':
				r->header_start = p;
				r->header_end = p;
				state_ = sw_almost_done;
				break;
			case '\n':
				r->header_start = p;
				r->header_end = p;
				goto done;
			case '\0':
				return HTTP_ERROR;

			default:
				r->header_start = p;
				state_ = sw_value;
				break;
			}
			break;
		case sw_value:
			switch (ch)
			{
			case ' ':
				r->header_end = p;
				state_ = sw_space_after_value;
				break;
			case '\r':
				r->header_end = p;
				state_ = sw_almost_done;
				break;
			case '\n':
				r->header_end = p;
				goto done;

			case '\0':
				return HTTP_ERROR;
			}
			break;

		case sw_space_after_value:
			switch (ch) {
			case ' ':
				break;
			case '\r':
				state_ = sw_almost_done;
				break;
			case '\n':
				goto done;
			case '\0':
				return HTTP_ERROR;
			default:
				state_ = sw_value;
				break;
			}
			break;

		case sw_almost_done:
			switch (ch) {
			case '\n':
				goto done;
			case '\r':
				break;
			default:
				return HTTP_ERROR;
			}
			break;

		case sw_header_almost_done:
			switch (ch) {
			case '\n':
				goto header_done;
			default:
				return HTTP_ERROR;
			}

		done:
			state_ = sw_start;
			string name(r->header_name_start, r->header_name_end);
			string val(r->header_start, r->header_end);
			r->headers_in.head_in[name] = val;
		}

	}
	buf->set_pos(p);
	r->sub_state = state_;
	return HTTP_AGAIN;

header_done:

	buf->set_pos(NULL);
	buf->retrieveUntil(p + 1);
	r->sub_state = sw_start;
	return HTTP_OK;
}

int httpRequest::http_read_request_body(Buffer* buf)
{
	string content_length = "content_length";
	auto it = r->headers_in.head_in.find(content_length);
	if (it == r->headers_in.head_in.end())
	{
		return HTTP_OK;
	}

	int rest = stoi(r->headers_in.head_in[content_length]);

	if (rest <= 0)return HTTP_OK;

	int len = buf->readableBytes();
	if (len >= rest)
	{
		r->request_body.body = string(buf->peek(), rest);
		buf->retrieve(rest);
		return HTTP_OK;
	}
	else
	{
		r->request_body.body += string(buf->peek(), len);
		rest -= len;
		buf->retrieveAll();
		return HTTP_AGAIN;
	}
}

int httpRequest::http_handle_read(Buffer* buf)
{
	int ret;
	enum 
	{
		STATE_PARSE_REQUEST_LINE = 1,
		STATE_PARSE_REQUEST_HEADER,
		STATE_PARSE_RECV_BODY,
		STATE_PARSE_FINSH
	};

	int state = r->state;


	while (true)
	{
		if (state == STATE_PARSE_REQUEST_LINE)
		{
			ret = http_parse_request_line(buf);

			if (ret == HTTP_AGAIN)
			{
				r->state = STATE_PARSE_REQUEST_LINE;
				return HTTP_AGAIN;
			}
			else if (ret == HTTP_OK)
			{
				state = STATE_PARSE_REQUEST_HEADER;
			}
			else if (ret == HTTP_ERROR)
			{
				goto error;
			}
		}
		if (state == STATE_PARSE_REQUEST_HEADER)
		{
			ret = http_parse_header_line(buf);

			if (ret == HTTP_AGAIN)
			{
				r->state = STATE_PARSE_REQUEST_HEADER;
				return HTTP_AGAIN;
			}
			else if (ret == HTTP_OK)
			{
				state = STATE_PARSE_RECV_BODY;
			}
			else if (ret == HTTP_ERROR)
			{
				goto error;
			}
		}
		if (state == STATE_PARSE_RECV_BODY)
		{
			ret = http_read_request_body(buf);

			if (ret == HTTP_AGAIN)
			{
				r->state = STATE_PARSE_RECV_BODY;
				return HTTP_AGAIN;
			}
			else if (ret == HTTP_OK)
			{
				state = STATE_PARSE_FINSH;
			}
			else if (ret == HTTP_ERROR)
			{
				goto error;
			}
		}
		if (state == STATE_PARSE_FINSH)
		{
			r->state = STATE_PARSE_REQUEST_LINE;

			cout << "HTTP: " << r->http_version << " " << "method: " << r->method << " " << "uri: " << r->uri << endl;

			for (auto it = r->headers_in.head_in.begin(); it != r->headers_in.head_in.end(); it++)
			{
				cout << it->first << " " << it->second << endl;
			}
			return HTTP_OK;
		}
	}
	
error:
	r->state = STATE_PARSE_REQUEST_LINE;
	return HTTP_ERROR;

}

int httpRequest::http_handle_write(Buffer*buf, int type)
{
	if (type == HTTP_ERROR)
	{
		string short_msg = " Not Found!";
		char send_buff[4096];
		string body_buff, header_buff;
		body_buff += "<html><title>哎~出错了</title>";
		body_buff += "<body bgcolor=\"ffffff\">";
		body_buff += to_string(404) + short_msg;
		body_buff += "<hr><em> Senwang's Web Server</em>\n</body></html>";

		header_buff += "HTTP/1.1 " + to_string(404) + short_msg + "\r\n";
		header_buff += "Content-Type: text/html\r\n";
		header_buff += "Connection: Close\r\n";
		header_buff += "Content-Length: " + to_string(body_buff.size()) + "\r\n";
		header_buff += "Server: Senwang's Web Server\r\n";;
		header_buff += "\r\n";

		buf->append(header_buff.c_str(), header_buff.size());
		buf->append(body_buff.c_str(), body_buff.size());
	}
	else
	{
		string short_msg = string("OK");
		short_msg = " " + short_msg;

		string body_buff, header_buff;
		body_buff += "hi senwang";
		body_buff += "<html><title>hi senwang</title>";
		body_buff += "<body bgcolor=\"ffffff\">";
		body_buff += to_string(200) + short_msg;
		body_buff += "<hr><em> SenWang's Web Server</em>\n</body></html>";

		header_buff += "HTTP/1.1 " + to_string(200) + short_msg + "\r\n";
		header_buff += "Content-type: text/html\r\n";
		header_buff += "Connection: Keep-Alive\r\n";
		header_buff += "Content-length: " + to_string(body_buff.size()) + "\r\n";
		header_buff += "\r\n";

		buf->append(header_buff.c_str(), header_buff.size());
		buf->append(body_buff.c_str(), body_buff.size());
	}
}