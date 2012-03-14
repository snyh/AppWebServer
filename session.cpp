#include "session.hpp"
#include "request_response.hpp"

#ifdef AWServer_DEBUG
#include <fstream>
#else
#include "content.hpp"
#endif

using namespace std;
using boost::asio::ip::tcp;

Session::Session(boost::asio::io_service& io_service, AWServer::Method m)
	:socket_(io_service),
	do_rpc_(m),
	is_valid_(true)
{
}
void Session::start()
{
  this->read_request();
}

void Session::read_request()
{
  auto s = shared_from_this();
  socket_.async_read_some(boost::asio::buffer(buffer_),
						   [=](const boost::system::error_code& e, size_t t){
						   s->handle_read(e, t);
						   });
}

void Session::handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
  if (!ec) {
	  boost::tribool result;
	  result = parse_(req_, buffer_.data(), buffer_.data()+bytes_transferred);
	  if (result) {
		  if (req_.uri == "/rpc.cgi") {
			  handle_rpc();
		  } else {
			  handle_file();
		  }
		  write_response();
	  } else if (!result) {
		  rep_ = stock_response(Response::bad_request);
		  write_response();
	  } else {
		  //need more data to determine the request
		  read_request();
	  }
  } else {
	  is_valid_ = false;
	  boost::system::system_error e(ec);
	  cout << "handle read " << e.what() << endl;
  }
}


void Session::write_response()
{
  auto s = shared_from_this();
  boost::asio::async_write(socket_, rep_.to_buffers(),
						   [=](const boost::system::error_code& e, size_t t){
						   s->handle_write(e);
						   });
}

void Session::handle_write(const boost::system::error_code& ec)
{
  if (!ec) {
	  boost::system::error_code ignored_ec;
	  socket_.shutdown(tcp::socket::shutdown_both, ignored_ec);
  } else {
	  is_valid_ = false;
	  boost::system::system_error e(ec);
	  cout << "handle_write " << e.what() << endl;
  }
}

namespace {
	string extension_to_mime(string& ext) {
		static struct mapping {
			const char* extension;
			const char* mime_type;
		} mappings[] = {
			  { "gif", "image/gif" },
			  { "htm", "text/html" },
			  { "html", "text/html" },
			  { "jpg", "image/jpeg" },
			  { "png", "image/png" },
			  { "js", "text/javascript"},
			  { "css", "text/css"},
			  { 0, 0 } // Marks end of list.
		};
		for (mapping* m = mappings; m->extension; ++m) {
			if (m->extension == ext) {
				return m->mime_type;
			}
		}
		return "text/plain";
	}


}

void Session::handle_rpc()
{
  AWServer::JSON r = do_rpc_(AWServer::s2j(req_.params));
  rep_.copy_content(AWServer::j2s(r));
  rep_.status = Response::ok;
  rep_.headers = {
		{"Content-Length", std::to_string(rep_.size())},
		{"Content-Type", "text/javascript"},
		{"Connection", "close"},

  };
}

void Session::handle_file()
{
  string request_path = req_.uri;
  size_t slash_pos = request_path.find_last_of("/");
  size_t dot_pos = request_path.find_last_of(".");
  string ext;
  if (slash_pos != string::npos && dot_pos > slash_pos) {
	  ext = request_path.substr(dot_pos + 1);
  }
#ifdef AWServer_DEBUG
  string full_path = "doc_root" + request_path;
  ifstream is(full_path.c_str(), ios::in | ios::binary);
  if (!is) {
	  rep_ = stock_response(Response::not_found);
	  return;
  }
  //TODO:MEMORY LEAK FIX
  static char buf[1024*1024*4];
  is.read(buf, sizeof(buf));
  rep_.set_content(buf, is.gcount());
#else
  auto p = _RC(request_path);
  rep_.set_content((char*)p.first, p.second);
  if (rep_.size() == -1) {
	  rep_ = stock_response(Response::not_found);
	  return;
  }
#endif

  rep_.status = Response::ok;
  rep_.headers = {
		{"Content-Length", std::to_string(rep_.size())},
		{"Content-Type", extension_to_mime(ext) + ";charset=utf-8"},
		{"Connection", "close"},
  };
}

