#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>

#include "jrpc.hpp"
namespace AWS {
class HTTPServer: public boost::noncopyable {
public:
  /// port为需要绑定的监听端口，默认随机绑定稍候使用
  /// port()函数得到绑定的端口, 或直接使用open_browser()打开游览器
  explicit HTTPServer(int port=0);
  ~HTTPServer();

  int port();
  void run();
  void stop();
  bool open_browser();

  void set_rpc(JSONPServer s);
private:
  class impl;
  std::unique_ptr<impl> pimpl;
};

}

#endif
