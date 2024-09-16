#pragma once

#include <sstream>
#include <locale>
#include <iostream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <map>
#include <boost/algorithm/string.hpp>
#include <pqxx/pqxx>
#include "../Spider/IniParser.h"
#include <algorithm>
#include <vector>

using namespace std;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class Server : public enable_shared_from_this<Server> {
public:
	void start();
	void ProcessRequest();
	void createResponseGet();
	void createResponsePost();
	http::request<http::dynamic_body> req;
	http::response<http::dynamic_body> res;
	pqxx::result SearchDB(vector<string> str);
	beast::flat_buffer buffer;
	tcp::socket socket_;
	Server(tcp::socket socket);
	void CheckDeadline();
	
	net::steady_timer deadline_{
		socket_.get_executor(), chrono::seconds(60) };
private:
	void ReadRequest();
	string host_, dbname_, user_, password_, port_;
};