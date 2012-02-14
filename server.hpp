#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>

#include "jrpc.hpp"

class Server: public boost::noncopyable {
public:
  explicit Server();
  ~Server();

  void run();
  void stop();

  void set_rpc(JRPC::Server s);
private:
  class impl;
  std::unique_ptr<impl> pimpl;
};

#endif
