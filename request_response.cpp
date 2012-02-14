#include "request_response.hpp"
#include <utility>
#include <string>
using namespace std;
namespace {
	bool is_char(int c)
	  {
		return c >= 0 && c <= 127;
	  }

	bool is_ctl(int c)
	  {
		return (c >= 0 && c <= 31) || (c == 127);
	  }

	bool is_tspecial(int c)
	  {
		switch (c)
		  {
		  case '(': case ')': case '<': case '>': case '@':
		  case ',': case ';': case ':': case '\\': case '"':
		  case '/': case '[': case ']': case '?': case '=':
		  case '{': case '}': case ' ': case '\t':
			return true;
		  default:
			return false;
		  }
	  }

	bool is_digit(int c)
	  {
		return c >= '0' && c <= '9';
	  }

	bool url_decode(const std::string& in, std::string& out) {
		out.clear();
		out.reserve(in.size());
		for (std::size_t i = 0; i < in.size(); ++i) {
			if (in[i] == '%') {
				if (i + 3 <= in.size()) {
					int value = 0;
					std::istringstream is(in.substr(i + 1, 2));
					if (is >> std::hex >> value) {
						out += static_cast<char>(value);
						i += 2;
					} else {
						return false;
					} 
				} else {
					return false;
				}
			} else if (in[i] == '+') {
				out += ' ';
			} else {
				out += in[i];
			}
		}
		return true;
	}
}

namespace status_strings {

	const std::string ok =
	  "HTTP/1.0 200 OK\r\n";
	const std::string no_content =
	  "HTTP/1.0 204 No Content\r\n";
	const std::string not_modified =
	  "HTTP/1.0 304 Not Modified\r\n";
	const std::string bad_request =
	  "HTTP/1.0 400 Bad Request\r\n";
	const std::string not_found =
	  "HTTP/1.0 404 Not Found\r\n";
	const std::string internal_server_error =
	  "HTTP/1.0 500 Internal Server Error\r\n";

	boost::asio::const_buffer to_buffer(Response::status_type status)
	  {
		switch (status)
		  {
		  case Response::ok:
			return boost::asio::buffer(ok);
		  case Response::no_content:
			return boost::asio::buffer(no_content);
		  case Response::not_modified:
			return boost::asio::buffer(not_modified);
		  case Response::bad_request:
			return boost::asio::buffer(bad_request);
		  case Response::not_found:
			return boost::asio::buffer(not_found);
		  case Response::internal_server_error:
			return boost::asio::buffer(internal_server_error);
		  default:
			return boost::asio::buffer(internal_server_error);
		  }
	  }

} // namespace status_strings

namespace misc_strings {

	const char name_value_separator[] = { ':', ' ' };
	const char crlf[] = { '\r', '\n' };

} // namespace misc_strings

std::vector<boost::asio::const_buffer> Response::to_buffers()
{
  std::vector<boost::asio::const_buffer> buffers;
  buffers.push_back(status_strings::to_buffer(status));
  for (std::size_t i = 0; i < headers.size(); ++i)
	{
	  Header& h = headers[i];
	  buffers.push_back(boost::asio::buffer(h.name));
	  buffers.push_back(boost::asio::buffer(misc_strings::name_value_separator));
	  buffers.push_back(boost::asio::buffer(h.value));
	  buffers.push_back(boost::asio::buffer(misc_strings::crlf));
	}
  buffers.push_back(boost::asio::buffer(misc_strings::crlf));
  if (is_copy) {
	  buffers.push_back(boost::asio::buffer(content2.data(), content2.size()));
  } else {
	  buffers.push_back(boost::asio::buffer(content, c_size));
  }
  return buffers;
}

namespace stock_replies {

	const char ok[] = "";
	const char no_content[] =
	  "<html>"
	  "<head><title>No Content</title></head>"
	  "<body><h1>204 Content</h1></body>"
	  "</html>";
	const char not_modified[] =
	  "<html>"
	  "<head><title>Not Modified</title></head>"
	  "<body><h1>304 Not Modified</h1></body>"
	  "</html>";
	const char bad_request[] =
	  "<html>"
	  "<head><title>Bad Request</title></head>"
	  "<body><h1>400 Bad Request</h1></body>"
	  "</html>";
	const char forbidden[] =
	  "<html>"
	  "<head><title>Forbidden</title></head>"
	  "<body><h1>403 Forbidden</h1></body>"
	  "</html>";
	const char not_found[] =
	  "<html>"
	  "<head><title>Not Found</title></head>"
	  "<body><h1>404 Not Found</h1></body>"
	  "</html>";
	const char internal_server_error[] =
	  "<html>"
	  "<head><title>Internal Server Error</title></head>"
	  "<body><h1>500 Internal Server Error</h1></body>"
	  "</html>";

	std::tuple<const char*, size_t> to_string(Response::status_type status)
	  {
		using namespace std;
		switch (status)
		  {
		  case Response::ok:
			return make_tuple(ok, sizeof(ok));
		  case Response::no_content:
			return make_tuple(no_content, sizeof(no_content));
		  case Response::bad_request:
			return make_tuple(bad_request, sizeof(bad_request));
		  case Response::not_found:
			return make_tuple(not_found, sizeof(not_found));
		  case Response::internal_server_error:
			return make_tuple(internal_server_error, sizeof(internal_server_error));
		  default:
			return make_tuple(internal_server_error, sizeof(internal_server_error));
		  }
	  }

} // namespace stock_replies

Response stock_response(Response::status_type status)
{
  Response rep;
  rep.status = status;
  auto p = stock_replies::to_string(status);
  rep.set_content(get<0>(p), get<1>(p));
  rep.headers.resize(2);
  rep.headers[0].name = "Content-Length";
  rep.headers[0].value = std::to_string(rep.size());
  rep.headers[1].name = "Content-Type";
  rep.headers[1].value = "text/html";
  return rep;
}


RequestParser::RequestParser()
: state_(method_start)
{
}

boost::tribool RequestParser::consume(Request& req, char input)
{
  switch (state_) {
	case method_start:
	  if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
		  return false;
	  } else {
		  state_ = method;
		  req.method.push_back(input);
		  return boost::indeterminate;
	  }
	case method:
	  if (input == ' ') {
		  state_ = uri;
		  return boost::indeterminate;
	  } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
		  return false;
	  } else {
		  req.method.push_back(input);
		  return boost::indeterminate;
	  }
	case uri_start:
	  if (is_ctl(input)) {
		  return false;
	  } else {
		  state_ = uri;
		  req.uri.push_back(input);
		  return boost::indeterminate;
	  }
	case uri:
	  if (input == ' ') {
		  state_ = http_version_h;
		  return boost::indeterminate;
	  } else if (is_ctl(input)) {
		  return false;
	  } else {
		  req.uri.push_back(input);
		  return boost::indeterminate;
	  }
	case http_version_h:
	  if (input == 'H') {
		  state_ = http_version_t_1;
		  return boost::indeterminate;
	  } else {
		  return false;
	  }
	case http_version_t_1:
	  if (input == 'T') {
		  state_ = http_version_t_2;
		  return boost::indeterminate;
	  } else {
		  return false;
	  }
	case http_version_t_2:
	  if (input == 'T') {
		  state_ = http_version_p;
		  return boost::indeterminate;
	  } else {
		  return false;
	  }
	case http_version_p:
	  if (input == 'P') {
		  state_ = http_version_slash;
		  return boost::indeterminate;
	  } else {
		  return false;
	  }
	case http_version_slash:
	  if (input == '/') {
		  req.http_version_major = 0;
		  req.http_version_minor = 0;
		  state_ = http_version_major_start;
		  return boost::indeterminate;
	  } else {
		  return false;
	  }
	case http_version_major_start:
	  if (is_digit(input)) {
		  req.http_version_major = req.http_version_major * 10 + input - '0';
		  state_ = http_version_major;
		  return boost::indeterminate;
	  } else {
		  return false;
	  }
	case http_version_major:
	  if (input == '.') {
		  state_ = http_version_minor_start;
		  return boost::indeterminate;
	  } else if (is_digit(input)) {
		  req.http_version_major = req.http_version_major * 10 + input - '0';
		  return boost::indeterminate;
	  } else {
		  return false;
	  }
	case http_version_minor_start:
	  if (is_digit(input)) {
		  req.http_version_minor = req.http_version_minor * 10 + input - '0';
		  state_ = http_version_minor;
		  return boost::indeterminate;
	  } else {
		  return false;
	  }
	case http_version_minor:
	  if (input == '\r') {
		  state_ = expecting_newline_1;
		  return boost::indeterminate;
	  } else if (is_digit(input)) {
		  req.http_version_minor = req.http_version_minor * 10 + input - '0';
		  return boost::indeterminate;
	  } else {
		  return false;
	  }
	case expecting_newline_1:
	  if (input == '\n') {
		  state_ = header_line_start;
		  return boost::indeterminate;
	  } else {
		  return false;
	  }
	case header_line_start:
	  if (input == '\r') {
		  state_ = expecting_newline_3;
		  return boost::indeterminate;
	  } else if (!req.headers.empty() && (input == ' ' || input == '\t')) {
		  state_ = header_lws;
		  return boost::indeterminate;
	  } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
		  return false;
	  } else {
		  req.headers.push_back(Header());
		  req.headers.back().name.push_back(input);
		  state_ = header_name;
		  return boost::indeterminate;
	  }
	case header_lws:
	  if (input == '\r') {
		  state_ = expecting_newline_2;
		  return boost::indeterminate;
	  } else if (input == ' ' || input == '\t') {
		  return boost::indeterminate;
	  } else if (is_ctl(input)) {
		  return false;
	  } else {
		  state_ = header_value;
		  req.headers.back().value.push_back(input);
		  return boost::indeterminate;
	  }
	case header_name:
	  if (input == ':') {
		  state_ = space_before_header_value;
		  return boost::indeterminate;
	  } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
		  return false;
	  } else {
		  req.headers.back().name.push_back(input);
		  return boost::indeterminate;
	  }
	case space_before_header_value:
	  if (input == ' ') {
		  state_ = header_value;
		  return boost::indeterminate;
	  } else {
		  return false;
	  }
	case header_value:
	  if (input == '\r') {
		  state_ = expecting_newline_2;
		  return boost::indeterminate;
	  } else if (is_ctl(input)) {
		  return false;
	  } else {
		  req.headers.back().value.push_back(input);
		  return boost::indeterminate;
	  }
	case expecting_newline_2:
	  if (input == '\n') {
		  state_ = header_line_start;
		  return boost::indeterminate;
	  } else {
		  return false;
	  }
	case expecting_newline_3:
	  return (input == '\n');
	default:
	  return false;
  }
}



bool RequestParser::deal_path(Request& req) 
{

  string request_path;
  if (!url_decode(req.uri, request_path)) {
	  return false;
  }
  size_t param_pos = request_path.find_first_of("?");
  if (param_pos != string::npos) {
	  string params = request_path.substr(param_pos+1);
	  request_path = request_path.substr(0, param_pos);
	  req.params = params;
  }
  if (request_path[request_path.size()-1] == '/')
	request_path += "index.htm";
  req.uri = request_path;
  return true;
}

boost::tribool RequestParser::operator()(Request& req, char* begin, char* end)
{
  while (begin != end) {
	  boost::tribool result = consume(req, *begin++);
	  if (result) {
		  result = deal_path(req);
		  return true;
	  } else if (!result){
		  return false;
	  }
  }
  return boost::indeterminate;
}
