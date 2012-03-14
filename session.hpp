#ifndef SESSION_HPP_
#define SESSION_HPP_

#include <boost/asio.hpp>
#include <memory>
#include <array>
#include <set>
#include "jrpc.hpp"
#include "request_response.hpp"

class Session: public std::enable_shared_from_this<Session>,
	private boost::noncopyable
{
public:
  explicit Session(boost::asio::io_service& io_service, AWServer::Method m);
  void start();
  boost::asio::ip::tcp::socket& socket() { return socket_; }
private:
  bool valid() const { return is_valid_; }

  void handle_file();
  void handle_rpc();

  void read_request();
  void write_response();
  void set_response(Response& rep) { rep_ = rep; }

  void handle_write(const boost::system::error_code& e);
  void handle_read(const boost::system::error_code& e,
				   std::size_t bytes_transferred);

  boost::asio::ip::tcp::socket socket_;
  std::array<char, 8192> buffer_;
  Request req_;
  Response rep_;
  RequestParser parse_;
  bool is_valid_;
  AWServer::Method do_rpc_;
};

typedef std::shared_ptr<Session> SessionPtr;
#endif
