#include "Spider.h"

//queue <string> s_queue;
queue<function<void()>> s_queue;

Spider::Spider(const string& startURL, const int& depth, const string& port) {
	this->start_url = startURL;
	this->depth_ = depth;
	this->port_ = port;
	findsLinks.emplace_back(start_url);
}

void Spider::startSpider() {
	work();
}
	/*for (size_t i = 0; i < size_thread; i++) {
		threads.push_back(thread(&Spider::thread_pool));
	}
	{
		lock_guard<mutex> lock(mtx);
		s_queue.push([&] {work(); });
		cond.notify_one();
	}
	
	for (auto& t : threads) {
		t.join();
	}
}

void Spider::thread_pool() {
	/*if (depth_ == 2) {
		this_thread::sleep_for(std::chrono::seconds(2));
		{
			std::lock_guard<std::mutex> lock(mtx);
			thread_finish = true;
			cond.notify_all();
			return;
		}

	}
	unique_lock<mutex> lock(mtx);
	if ((thread_finish == false) || !s_queue.empty()) {
		if (s_queue.empty()) {
			cond.wait(lock);
		}
		else {
			auto queue = s_queue.front();		
			s_queue.pop();
			lock.unlock();
			queue();
			lock.lock();
		}
	}*/
//}

void Spider::work(){
	    usedLinks.push_back(findsLinks.front());
		string url = findsLinks.front();
		findsLinks.erase(findsLinks.begin());
		string HTML = loadHTTP(url);
		ParserURL(HTML);
}

string Spider::loadHTTP(const string& url) {
	try {
		vector <string> datas = LinkDatas(url);
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
			}
				tcp::resolver resolver(ioc);

				auto const results = resolver.resolve({ datas[1], port_ });
				get_lowest_layer(stream).connect(results);
				http::request<http::string_body> req{ http::verb::get, datas[2], 11 };
				req.set(http::field::host, datas[1]);
				req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
				stream.handshake(ssl::stream_base::client);
				http::write(stream, req);
				beast::flat_buffer buffer;
				http::response<http::dynamic_body> res;
				http::read(stream, buffer, res);
				string sBody = beast::buffers_to_string(res.body().data());

				beast::error_code ec;
				stream.shutdown(ec);
				if (ec == net::error::eof) {
					ec = {};
				}

				if (ec) {
					throw beast::system_error{ ec };
				}
				//cout << res << endl;
				return sBody;
		}
		else {
			tcp::resolver resolver(ioc);
			beast::tcp_stream stream(ioc);
			auto const results = resolver.resolve(datas[1], port_);
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
			stream.socket().shutdown(tcp::socket::shutdown_both, ec);

			if (ec && ec != beast::errc::not_connected)
				throw beast::system_error{ ec };
			return sBody;
		}
	}
    catch (exception const& e)
    {
		cerr << "Error: " << e.what() << endl;
    }
}

vector<string> Spider::LinkDatas(const string& url) {
	vector<string> datas;
	string line;
	auto slesh2 = url.find("//");
	datas.push_back(url.substr(0, slesh2));
	line = url.substr(slesh2 + 2, url.length());
	auto slesh = line.find("/");
	datas.push_back(line.substr(0, slesh));
	datas.push_back(line.substr(slesh, line.length()));
	return datas;
}


int Spider::ParserURL(const string& html) {
	try {
		//const char* html = "<!DOCTYPE html><html><body><div>Content 1</div><p>Paragraph</p><div>Content 2</div></body></html>";
		htmlDocPtr doc = htmlReadMemory(html.c_str(), html.length(), nullptr, nullptr, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
		if (doc == NULL) {
			cerr << "Failed to parse HTML" << std::endl;
			return -1;
		}
		xmlNodePtr root = xmlDocGetRootElement(doc);
		if (root == NULL) {
			cout << "Empty document\n";
			xmlFreeDoc(doc);
			return -1;
		}
		xmlXPathContextPtr context = xmlXPathNewContext(doc);
		xmlXPathObjectPtr html_tags = xmlXPathEvalExpression((const xmlChar*)"//div/text()", context);
		if (html_tags == NULL) {
			cout << "Failed to evaluate XPath expression\n";
			xmlXPathFreeContext(context);
			xmlFreeDoc(doc);
			return -1;
		}
		xmlNodeSetPtr nodes = html_tags->nodesetval;
		for (int i = 0; i < nodes->nodeNr; i++) {
			xmlNodePtr tag = nodes->nodeTab[i];
			xmlXPathSetContextNode(tag, context);
			xmlUnlinkNode(tag);
			xmlFree(tag);
			tag = NULL;
		}
		/*xmlXPathObjectPtr html_urls = xmlXPathEvalExpression((const xmlChar*)"//a/@href", context);

		//xmlNodePtr html_url = html_urls->nodesetval;
			//xmlXPathEvalExpression((xmlChar*)"/html/body//a", context)->nodesetval->nodeTab[0];
		if (html_urls == NULL) {
			cout << "Failed to evaluate XPath expression\n";
			xmlXPathFreeContext(context);
			xmlFreeDoc(doc);
			return -1;
		}
		for (int i = 0; i < html_urls->nodesetval->nodeNr; i++) {
			xmlNodePtr html_url = html_urls->nodesetval->nodeTab[i];
			string url = string(reinterpret_cast<char*>(xmlGetProp(html_url, (xmlChar*)"href")));
			//xmlDocDumpMemory((xmlDocPtr)notekey, &s, &size)
			findsLinks.push_back(url);
			xmlFree(html_url);
			html_url = NULL;
		}*/
		xmlXPathFreeObject(html_tags);
		//xmlXPathFreeObject(html_urls);
		xmlXPathFreeContext(context);
		xmlChar* html_buffer;
		int buffer_size;
		xmlDocDumpFormatMemory(doc, &html_buffer, &buffer_size, 1);
		//printf("%s", (char*)html_buffer);
		cout << html_buffer;
		// Очистка памяти
		xmlFree(html_buffer);
		xmlFreeDoc(doc);
		xmlCleanupParser();
	}
	catch (exception const& e)
	{
		cerr << "Error: " << e.what() << endl;
	}
	return 0;
}

void Spider::remove_node(xmlNodePtr node) {
	if (node != NULL) {
		xmlUnlinkNode(node);
		xmlFreeNode(node);
	}
}
/*
inline void Spider::ltrim(string& s) {
	s.erase(s.begin(), find_if(s.begin(), s.end(), [](int ch) {
		return !isspace(ch);
		}));
}*/

void Spider::LoadtoDB() {

}