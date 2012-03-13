#ifndef __JRPC_HPP_
#define __JRPC_HPP_

#include <functional>
#include "json/json.h"
#include <string>

namespace AWS {
	typedef Json::Value JSON;
	typedef std::function<JSON(const JSON&)> Method;
	struct Service {
		std::string name;
		std::map<std::string, Method> methods;
	};
	JSON s2j(const std::string&);
	std::string j2s(const JSON&);
	inline JSON stock_ok(const JSON& id, const JSON& result) {
		JSON root;
		root["result"] = result;
		root["id"] = id;
		root["error"]; // for null
		return root;
	}
	enum error_code {
		illegal_service = 1,
		service_not_found = 2,
		class_not_found = 3,
		method_not_found = 4,
		parameter_mismatch = 5,
		permission_denied = 6,
		not_found_file
	};

	inline JSON stock_error(error_code c, int id) {
		JSON root;
		root["result"];
		root["error"]["origin"] = 1;
		root["error"]["code"] = c;
		return root;
	}


	class JSONPServer {
	public:
	  void install_service(Service& s);
	  void remove_service(const std::string& name);
	  JSON do_rpc(const JSON&);
	private:
	  std::map<std::string, Service> services_;
	};

}

#endif
