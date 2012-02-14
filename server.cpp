#include "server.hpp"
#include "session.hpp"
#include <boost/asio.hpp>
#include <set>
#include <vector>

using boost::asio::ip::tcp;
using namespace std;


class Server::impl {
public:
	impl();
	void run() { io_service_.run(); }
	void stop() { acceptor_.close(); }

	void handle_accept(const boost::system::error_code& ec);

	boost::asio::io_service io_service_;
	tcp::acceptor acceptor_;
	JRPC::Server rpc_s_;
	SessionPtr new_session_;
};

Server::Server() :pimpl(new impl) { }
Server::~Server() {}
void Server::run() { pimpl->run(); }
void Server::stop() { pimpl->run(); }

void Server::set_rpc(JRPC::Server s)
{
  this->pimpl->rpc_s_ = s;
}


Server::impl::impl()
	:io_service_(),
	acceptor_(io_service_),
	rpc_s_(),
	new_session_(new Session(io_service_, 
							 bind(&JRPC::Server::do_rpc, &rpc_s_, 
								  std::placeholders::_1)))
{
  tcp::endpoint endpoint(tcp::v4(), 8080);
  acceptor_.open(tcp::v4());
  acceptor_.bind(endpoint);
  acceptor_.listen();

  acceptor_.async_accept(new_session_->socket(),
						 bind(&Server::impl::handle_accept, this,
							  std::placeholders::_1));
}


void Server::impl::handle_accept(const boost::system::error_code& ec)
{
  if (!ec) {
	  new_session_->start();
	  new_session_.reset(new Session(io_service_,
									 bind(&JRPC::Server::do_rpc, &rpc_s_,
										  std::placeholders::_1)));

	  acceptor_.async_accept(new_session_->socket(),
							 bind(&Server::impl::handle_accept,
								  this, std::placeholders::_1));
  }
}
