#include "jrpc.hpp"

namespace AWS {

	void JSONPServer::install_service(Service& s) {
		this->services_.insert(make_pair(s.name, s));
	}
	void JSONPServer::remove_service(const std::string& name) {
		this->services_.erase(name);
	}

	JSON JSONPServer::do_rpc(const JSON& req) {
		//std::cout << "RPC::Receive:\n" << j2s(req) << std::endl;
		if (!req.isMember("id") || !req["id"].isInt())
		  return stock_error(illegal_service, 0);

		//fetch the service name and find it in this->services_.
		if (!req.isMember("service") || !req["service"].isString())
		  return stock_error(illegal_service, req["id"].asInt());
		auto s = this->services_.find(req["service"].asString());
		if (s == this->services_.end())
		  return stock_error(service_not_found, req["id"].asInt());

		//fetch the method name and find it in this->services_.
		if (!req.isMember("method") || !req["method"].isString())
		  return stock_error(illegal_service, req["id"].asInt());
		auto m = s->second.methods.find(req["method"].asString());
		if (m == s->second.methods.end())
		  return stock_error(method_not_found, req["id"].asInt());

		//fetch the params name and find it in this->services_.
		if (!req.isMember("params") || !req["params"].isObject())
		  return stock_error(parameter_mismatch, req["id"].asInt());
		JSON p = req["params"];
		//return JRPC::stock_ok(req["id"], m->second(p));
		JSON re = stock_ok(req["id"], m->second(p));
		//std::cout << "RPC::Send:\n" << re << std::endl;
		return re;
	}


	JSON s2j(const std::string& p) {
		Json::Value v;
		Json::Reader r;
		if (!r.parse(p, v))
		  std::clog << "JSON PARSE ERROR!(" << p << ")\n";
		return v;
	}

	std::string j2s(const JSON& v) {
		return v.toStyledString();
	}
}
