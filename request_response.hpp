#ifndef REQUESTRESPONSE__
#define REQUESTRESPONSE__

#include <string>
#include <vector>
#include <boost/logic/tribool.hpp>
#include <boost/asio.hpp>

struct Header {
	std::string name;
	std::string value;
};
struct Request {
	std::string method;
	std::string uri;
	std::vector<Header> headers;
	std::string params;
	int http_version_major;
	int http_version_minor;
};

struct Response {
	enum status_type {
		ok = 200,
		no_content = 204,
		not_modified = 304,
		bad_request = 400,
		not_found = 404,
		internal_server_error = 500,
	} status;

	void copy_content(const std::string& s) {
		content2 = s;
		c_size = s.size();
		is_copy = true;
	}
	void set_content(const char* c, size_t s) {
		content = c;
		c_size = s;
		is_copy = false;
	}

	std::vector<boost::asio::const_buffer> to_buffers();

	std::vector<Header> headers;
	size_t size() { return c_size; }
private:
	bool is_copy;
	const char* content;
	size_t c_size;
	std::string content2;
};

Response stock_response(Response::status_type s);


class RequestParser
{
public:
  RequestParser();

  boost::tribool operator()(Request& req, char* begin, char* end);

private:
  boost::tribool consume(Request& req, char input);
  bool deal_path(Request& req);

  /// The current state of the parser.
  enum state
  {
    method_start,
    method,
    uri_start,
    uri,
    http_version_h,
    http_version_t_1,
    http_version_t_2,
    http_version_p,
    http_version_slash,
    http_version_major_start,
    http_version_major,
    http_version_minor_start,
    http_version_minor,
    expecting_newline_1,
    header_line_start,
    header_lws,
    header_name,
    space_before_header_value,
    header_value,
    expecting_newline_2,
    expecting_newline_3
  } state_;
};

#endif
