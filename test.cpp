#include "server.hpp"
#include <string>
#include <thread>

using namespace std;

void rpc_init(AWS::JSONPServer&);

int main()
{
  AWS::JSONPServer rpc;
  rpc_init(rpc);

  AWS::HTTPServer s;
  s.set_rpc(rpc);
  s.open_browser();
  s.run();
  return 0;
}

void rpc_init(AWS::JSONPServer& rpc)
{
  typedef const AWS::JSON MP;
  enum {REMOTE, LOCAL, DOWNLOAD};

  AWS::Service filemanager;
  filemanager.name = "filemanager";
  filemanager.methods = {
		{
		  "file_list", [](MP& j){
			  AWS::JSON result;
			  AWS::JSON node;
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
			  return AWS::stock_ok(j["id"], result);
		  }
		},
		{
		  "add_file", [](MP& j){
			  AWS::JSON node;
			  node["name"] = "new.htm";
			  node["size"] = .023;
			  node["status"] = 0;
			  node["type"] = 0;
			  node["progress"] = 1;
			  return AWS::stock_ok(j["id"], node);
		  }
		},
  };
  rpc.install_service(filemanager);
}
