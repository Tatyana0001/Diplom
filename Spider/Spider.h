#pragma once
#include <iostream>
#include <string>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <regex>
#include <mutex>
#include <unordered_set>
#include <vector>
#include <thread>
#include <queue>
#include <condition_variable>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ssl.hpp>
#include <openssl/ssl.h> 
#include "libxml/HTMLparser.h"
#include "libxml/xpath.h"

using namespace std;
namespace beast = boost::beast;     
namespace http = beast::http;       
namespace net = boost::asio;        
using tcp = net::ip::tcp;           
namespace ssl = boost::asio::ssl;

class Spider {
private:
	string start_url;
	int depth_;
	string port_;
	net::io_context ioc;
	//regex rUri{"^(?:(https?)://)([^/]+)(/.*)?"};
	mutex mtx;
	//smatch match;
	vector<string>findsLinks;
	vector<string>usedLinks;
	vector<thread> threads;
	const int size_thread = thread::hardware_concurrency();
	condition_variable cond;
	bool thread_finish = false;
public:
	Spider(Spider const&) = delete;
	Spider& operator=(Spider const&) = delete;
	Spider(const string& startURL, const int& depth, const string& port);
	void startSpider();
	int ParserURL(const string& html);
	void LoadtoDB();
	string loadHTTP(const string& url);
	vector<string> LinkDatas(const string& url);
	void remove_node(xmlNodePtr node);
	void work();
	void thread_pool();
};