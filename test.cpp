#include <AWServer.hpp>
#include <string>
#include <thread>

using namespace std;

void rpc_init(AWServer::JSONPServer&);

int main()
{
  AWServer::JSONPServer rpc;
  rpc_init(rpc);

  AWServer::HTTPServer s;
  s.set_rpc(rpc);
  s.open_browser();
  s.run();
  return 0;
}

void rpc_init(AWServer::JSONPServer& rpc)
{
  typedef const AWServer::JSON MP;
  enum {REMOTE, LOCAL, DOWNLOAD};

  AWServer::Service filemanager;
  filemanager.name = "filemanager";
  filemanager.methods = {
		{
		  "file_list", [](MP& j){
			  AWServer::JSON result;
			  AWServer::JSON node;
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
			  return AWServer::stock_ok(j["id"], result);
		  }
		},
		{
		  "add_file", [](MP& j){
			  AWServer::JSON node;
			  node["name"] = "new.htm";
			  node["size"] = .023;
			  node["status"] = 0;
			  node["type"] = 0;
			  node["progress"] = 1;
			  return AWServer::stock_ok(j["id"], node);
		  }
		},
  };
  rpc.install_service(filemanager);
}
