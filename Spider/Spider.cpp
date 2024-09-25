#include "Spider.h"

queue <string> s_queue;

Spider::Spider(const string& startURL, const int& depth, const string& port, IniParser& IniParser) {
	this->start_url = startURL;
	this->depth_ = depth;
	this->port_ = port;
	s_queue.emplace(start_url);
	db["host"] = IniParser.get_data("host");
	db["port"] = IniParser.get_data("port");
	db["dbname"] = IniParser.get_data("dbname");
	db["user"] = IniParser.get_data("user");
	db["password"] = IniParser.get_data("password");
}

void Spider::startSpider() {
	int size_queue;
	while (depth_ > 0) {
		depth_--;
		auto work = [&](int& size) {
			while (size > 0) {
				unique_lock<mutex> lock(mtx);
				size--;
				auto link = s_queue.front();
				usedLinks.push_back(link);
				s_queue.pop();
				
				string HTML = loadHTTP(link);
				lock.unlock();
				ParserURL(HTML, link);
			}
		};
		size_queue = s_queue.size();
		if (s_queue.size() == 1) {
			work(size_queue);
		}
		else if (s_queue.size() > 1) {
			for (size_t i = 0; i < size_thread; i++) {
				threads.emplace_back(work, ref(size_queue));
				size_queue--;
			}
			
		}
	}
	for (auto& t : threads) {
		cond.notify_all();
		t.join();
	}
}

string Spider::loadHTTP(const string& link) {
	try {
		net::io_context ioc;
		vector <string> datas = LinkDatas(link);
		if (datas[0] == "https:") {
			ssl::context ctx(ssl::context::tlsv12_client);
			ctx.set_default_verify_paths();
			beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);
			stream.set_verify_mode(ssl::verify_none);
			stream.set_verify_callback([] (bool preverified, ssl::verify_context& ctx) {
				return true;
			});
			if (!SSL_set_tlsext_host_name(stream.native_handle(), datas[1].c_str()))
			{
				beast::error_code ec{ static_cast<int>(::ERR_get_error()), net::error::get_ssl_category() };
				throw beast::system_error{ ec };
				return "";
			}
			tcp::resolver resolver(ioc);
			auto const results = resolver.resolve({ datas[1], "https" });
			get_lowest_layer(stream).connect(results);
			get_lowest_layer(stream).expires_after(chrono::seconds(30));
			http::request<http::empty_body> req{ http::verb::get, datas[2], 11 };
			req.set(http::field::host, datas[1]);
			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
			stream.handshake(ssl::stream_base::client);
			http::write(stream, req);
			beast::flat_buffer buffer;
			http::response<http::dynamic_body> res;
			http::read(stream, buffer, res);
			string sBody = beast::buffers_to_string(res.body().data());

			beast::error_code ec;
			if (ec == ssl::error::stream_truncated) {
				return sBody;
			}
			if (ec == net::error::eof) {
				ec = {};
				return sBody;
			}
			if (ec) {
				throw beast::system_error{ ec };
				return sBody;
			}
			stream.shutdown(ec);
			
			return sBody;
		}
		else {
			tcp::resolver resolver(ioc);
			beast::tcp_stream stream(ioc);
			auto const results = resolver.resolve(datas[1], "http");
			stream.connect(results);
			http::request<http::string_body> req{ http::verb::get, datas[2], 10};
			req.set(http::field::host, datas[1]);
			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
			http::write(stream, req);
			beast::flat_buffer buffer;
			http::response<http::dynamic_body> res;
			http::read(stream, buffer, res);
			string sBody = beast::buffers_to_string(res.body().data());
			
			beast::error_code ec;
			if (ec == ssl::error::stream_truncated) {
				return sBody;
			}
			if (ec == net::error::eof) {
				ec = {};
				return sBody;
			}
			if (ec) {
				throw beast::system_error{ ec };
				return sBody;
			}
			stream.socket().shutdown(tcp::socket::shutdown_both, ec);
			return sBody;
		}
	}
	
    catch (exception const& e)
    {
		cerr << "Error: " << e.what() << endl;
		return "0";
    }
}

vector<string> Spider::LinkDatas(const string& link) {
	vector<string> datas;
	string line;
	auto slesh2 = link.find("//");
	datas.push_back(link.substr(0, slesh2));
	line = link.substr(slesh2 + 2, link.length());
	auto slesh = line.find("/");
	if (slesh > 100 ) {
		datas.push_back(line.substr(0, line.length()));
		datas.push_back("");
	}
	else {
		datas.push_back(line.substr(0, slesh));
		datas.push_back(line.substr(slesh, line.length()));
	}
	return datas;
}


void Spider::ParserURL(const string& html, const string& link) {
	try {
		htmlDocPtr doc = htmlReadMemory(html.c_str(), html.length(), nullptr, nullptr, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
		if (doc == NULL) {
			auto x = link;
			cerr << "Failed to parse HTML" << endl;
			return;
		}
		
		xmlNodePtr root = xmlDocGetRootElement(doc);
		if (root == NULL) {
			cout << "Empty document\n";
			xmlFree(doc);
			return;
		}
		string result = "";
		ExtractText(root, result);
		DeletePunct(result);
		xmlXPathContextPtr context = xmlXPathNewContext(doc);
		xmlXPathObjectPtr html_urls = xmlXPathEvalExpression((const xmlChar*)"//a[@href]", context);
		if (html_urls == NULL) {
			cout << "Failed to evaluate XPath expression\n";
			xmlXPathFreeContext(context);
			xmlFree(doc);
			return;
		}
		for (int i = 0; i < html_urls->nodesetval->nodeNr; i++) {
			xmlNodePtr html_url = html_urls->nodesetval->nodeTab[i];
			string url = string(reinterpret_cast<char*>(xmlGetProp(html_url, (xmlChar*)"href")));
			auto l = url.find("#");
			if (l < 150) {
				url = url.substr(0, l);
			}
			auto s = url.find("//");
			if (s > 100) {
				vector <string> datas = LinkDatas(link);
				url = datas[0] + "//" + datas[1] + url;
			}
			bool usedURL = false;
			for (const auto& i : findsLinks) {
					if (i == url){
						usedURL = true;
					}
			}
			for (const auto& j : usedLinks) {
				if (j == url) {
					usedURL = true;
				}
			}
			if (usedURL == false) {
				findsLinks.emplace_back(url);
				s_queue.emplace(url);
			}
			xmlFree(html_url);
			html_url = NULL;
		}
		
		xmlXPathFreeObject(html_urls);
		xmlXPathFreeContext(context);
		xmlFree(doc);
		xmlCleanupParser();
		LoadtoDB(result, link);
		finish = true;
	} 
	catch (exception const& e)
	{
		cerr << "Error: " << e.what() << endl;
	}
}

void Spider::ExtractText(xmlNode* node, string& result) {
	for (xmlNode* curnode = node; curnode; curnode = curnode->next) {
		if (curnode->type == XML_TEXT_NODE) {
			xmlChar* text = xmlNodeGetContent(curnode);
			if (nullptr != text) {
				result += reinterpret_cast<const char*>(text);
				result += " ";
			}
			xmlFree(text);
		}
		ExtractText(curnode->children, result);
	}
}

string Spider::DeletePunct(string& result) {
	boost::replace_all(result, "\n", " ");
	boost::replace_all(result, "\t", " ");
	boost::replace_all(result, "\"", " ");
	boost::replace_all(result, "=", " ");
	boost::replace_all(result, ".", " ");
	boost::replace_all(result, ",", " ");
	boost::replace_all(result, "-", " ");
	boost::replace_all(result, "_", " ");
	boost::replace_all(result, "+", " ");
	boost::replace_all(result, "*", " ");
	boost::replace_all(result, "[", " ");
	boost::replace_all(result, "]", " ");
	boost::replace_all(result, "(", " ");
	boost::replace_all(result, ")", " ");
	boost::replace_all(result, ";", " ");
	boost::replace_all(result, ":", " ");
	boost::replace_all(result, "/", " ");
	boost::replace_all(result, "\\", " ");
	boost::replace_all(result, ">", " ");
	boost::replace_all(result, "<", " ");
	boost::replace_all(result, "%", " "); 
	boost::replace_all(result, "#", " ");
	boost::replace_all(result, "'", " ");
	boost::replace_all(result, "$", " ");
	boost::replace_all(result, "?", " ");
	boost::replace_all(result, "!", " ");
	for (int i = 0; i < 10; i++) {
		string x = to_string(i);
		boost::replace_all(result, x, "");
	}
	while (result.find("  ") != string::npos) {
		boost::replace_all(result, "  ", " ");
	}
	return result;
}

void Spider::LoadtoDB(string& result, const string& link) {
	map<string, int> words;
	istringstream i(result);
	string word;
	while (i >> word) {
		if ((word.length() < 33) && (word.length() > 3)) {
			for (int i = 0; i < word.length(); i++) {
				if (word[i] >= 'A' && word[i] <= 'Z') {
					word[i] += 32;
				}
			}
			words[word]++;
		}
	}
	unique_lock<mutex> lock(mtx);
	database.SaveDatasToDB(words, link, db);
	lock.unlock();
}