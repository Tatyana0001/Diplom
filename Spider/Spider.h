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
#include <boost/algorithm/string.hpp>
#include <mutex>
#include <vector>
#include <thread>
#include <queue>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ssl.hpp>
#include <openssl/ssl.h> 
#include "libxml/HTMLparser.h"
#include "libxml/xpath.h"
#include <map>
#include "Database.h"
#include "IniParser.h"

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
	mutex mtx;
	vector<string>usedLinks;
	const int size_thread = thread::hardware_concurrency();
	vector<thread> threads;
	condition_variable cond;
	bool finish = false;
public:
	Spider(Spider const&) = delete;
	Spider& operator=(Spider const&) = delete;
	Spider(const string& startURL, const int& depth, const string& port, IniParser& Iniparser);
	map<string, string> db;
	Database database;
	void startSpider();
	void ParserURL(const string& html, const string& link);
	void LoadtoDB(string& result, const string& link);
	string loadHTTP(const string& link);
	vector<string> LinkDatas(const string& link);
	void ExtractText(xmlNode* node, string& result);
	string DeletePunct(string& result);
};