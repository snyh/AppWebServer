#define AWS_DEBUG
#include "server.hpp"
#include <string>
#include <thread>

using namespace std;

void rpc_init(JRPC::Server&);

int main()
{
  JRPC::Server rpc;
  rpc_init(rpc);

  Server s;
  s.set_rpc(rpc);
  s.run();
  return 0;
}

void rpc_init(JRPC::Server& rpc)
{
  typedef const JRPC::JSON MP;
  enum {REMOTE, LOCAL, DOWNLOAD};

  JRPC::Service filemanager;
  filemanager.name = "filemanager";
  filemanager.methods = {
		{
		  "file_list", [](MP& j){
			  JRPC::JSON result;
			  JRPC::JSON node;
			  node["name"] = "index.htm";
			  node["size"] = .023;
			  node["status"] = 0;
			  node["type"] = 0;
			  node["progress"] = 1;
			  result.append(node);

			  node["name"] = "war3.124.rar";
			  node["size"] = 1535;
			  node["status"] = 1;
			  node["type"] = 0;
			  node["progress"] = 0.54;
			  result.append(node);

			  node["name"] = "dc_root";
			  node["size"] = 12;
			  node["status"] = 2;
			  node["type"] = 1;
			  node["progress"] = 0;
			  result.append(node);
			  return JRPC::stock_ok(j["id"], result);
		  }
		},
		{
		  "add_file", [](MP& j){
			  JRPC::JSON node;
			  node["name"] = "new.htm";
			  node["size"] = .023;
			  node["status"] = 0;
			  node["type"] = 0;
			  node["progress"] = 1;
			  return JRPC::stock_ok(j["id"], node);
		  }
		},
  };
  rpc.install_service(filemanager);
}
