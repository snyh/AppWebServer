#include "server.hpp"
#include "session.hpp"
#include "DetectBrowser.hpp"
#include <boost/asio.hpp>
#include <set>
#include <vector>

namespace AWS {
	using boost::asio::ip::tcp;
	using namespace std;


	class HTTPServer::impl {
	public:
	  impl(int p);
	  void run() { io_service_.run(); }
	  void stop() { acceptor_.close(); }

	  void handle_accept(const boost::system::error_code& ec);

	  boost::asio::io_service io_service_;
	  tcp::acceptor acceptor_;
	  JSONPServer rpc_s_;
	  SessionPtr new_session_;
	  int port_;
	};

	HTTPServer::HTTPServer(int p) :pimpl(new impl(p)) {}
	HTTPServer::~HTTPServer() {}
	void HTTPServer::run() { pimpl->run(); }
	void HTTPServer::stop() { pimpl->run(); }
	int HTTPServer::port() { return pimpl->port_; }
	bool HTTPServer::open_browser() { 
		cout << "test:" << pimpl->port_ << endl;
		return Browser::open(pimpl->port_); 
	}

	void HTTPServer::set_rpc(JSONPServer s)
	  {
		this->pimpl->rpc_s_ = s;
	  }


	HTTPServer::impl::impl(int p)
	  :io_service_(),
	  acceptor_(io_service_),
	  rpc_s_(),
	  port_(p),
	  new_session_(new Session(io_service_, 
							   bind(&JSONPServer::do_rpc, &rpc_s_, 
									std::placeholders::_1)))
	{
	  //tcp::endpoint endpoint(8080);
	  acceptor_.open(tcp::v4());
	  acceptor_.bind(tcp::endpoint(tcp::v4(), port_));
	  acceptor_.listen();
	  port_ = acceptor_.local_endpoint().port();
	  cout << "Port:" <<  port_ << endl;

	  acceptor_.async_accept(new_session_->socket(),
							 bind(&HTTPServer::impl::handle_accept, this,
								  std::placeholders::_1));
	}


	void HTTPServer::impl::handle_accept(const boost::system::error_code& ec)
	  {
		if (!ec) {
			new_session_->start();
			new_session_.reset(new Session(io_service_,
										   bind(&JSONPServer::do_rpc, &rpc_s_,
												std::placeholders::_1)));

			acceptor_.async_accept(new_session_->socket(),
								   bind(&HTTPServer::impl::handle_accept,
										this, std::placeholders::_1));
		}
	  }
}
