#include "Server.h"


void Server::start() {
    ReadRequest();
    CheckDeadline();
}

void Server::ReadRequest(){
    auto self = shared_from_this();
    http::async_read(
        socket_,
        buffer,
        req,
        [self](beast::error_code ec,
            size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);
            if (!ec)
                self->ProcessRequest();
        });
    
}

Server::Server(tcp::socket socket, IniParser* IniParser) : socket_(move(socket)), IniParser_{ IniParser } {}



void Server::ProcessRequest() {
    try {
            res.version(req.version());
            res.keep_alive(false);

            switch (req.method()) {
            case http::verb::get: 
                    res.result(http::status::ok);
                    res.set(http::field::server, "Beast");
                    createResponseGet();
                    break;
            case http::verb::post: 
                res.result(http::status::ok);
                res.set(http::field::server, "Beast");
                createResponsePost();
                break;
            default: 
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "text/plain");
                beast::ostream(res.body())
                    << "Invalid request-method '"
                    << string(req.method_string())
                    << "'";
                break;
            }
            auto self = shared_from_this();

            res.content_length(res.body().size());

            http::async_write(socket_, res, [self](beast::error_code ec, size_t) {
                self->socket_.shutdown(tcp::socket::shutdown_send, ec);
                self->deadline_.cancel();
                });
    }
    catch (exception const& e){
        cerr << "Error: " << e.what() << "\n";
    }
}

void Server::createResponseGet() {
    res.set(http::field::content_type, "text/html");
    beast::ostream(res.body())
        << "<html>\n"
        << "<body>\n"
        << "<center>\n"
        << "<h1 style=\"color:#FF0000\">Search Server </h1>\n"
        << "<form action=\"/\" method=\"post\">\n"
        << "    <label for=\"search\">Enter your request:</label><br>\n"
        << "    <input type=\"text\" id=\"search\" name=\"search\"><br>\n"
        << "    <input style=\"background:green\" type=\"submit\" value=\"Search\">\n"
        << "</form>\n"
        << "</center>\n"
        << "</body>\n"
        << "</html>\n";
}

void Server::createResponsePost() {
    auto& body = this->req.body();
    auto body_str = beast::buffers_to_string(body.data());
    cout << "receive body is " << body_str << "\n";
    auto post = body_str.find('=');
    if (post > 500) {
        res.result(http::status::not_found);
        res.set(http::field::content_type, "text/plain");
        beast::ostream(res.body()) << "File not found\r\n";
        return;
    }
    string key = body_str.substr(0, post);
    string value = body_str.substr(post + 1);
    if (key != "search") {
        res.result(http::status::not_found);
        res.set(http::field::content_type, "text/plain");
        beast::ostream(res.body()) << "File not found\r\n";
        return;
    }
    boost::replace_all(value, ",", " ");
    boost::replace_all(value, ".", " ");
    boost::replace_all(value, "-", " ");
    boost::replace_all(value, ":", " ");
    boost::replace_all(value, "=", " ");
    boost::replace_all(value, "(", " ");
    boost::replace_all(value, ")", " ");
    boost::replace_all(value, "!", " ");
    boost::replace_all(value, "?", " ");
    boost::replace_all(value, "+", " ");
    vector<string> str;
    istringstream i(value);
    string word;
    while (i >> word) {
        if ((word.length() < 33) && (word.length() > 3)) {
            for (int i = 0; i < word.length(); i++) {
                if (word[i] >= 'A' && word[i] <= 'Z') {
                    word[i] += 32;
                }
            }
            str.push_back(word);
        }
    }
    vector<pair<string, int>> DBRes = SearchDB(str);
    res.set(http::field::content_type, "text/html");
    beast::ostream(res.body()) << "<html>\n"
        << "<head><meta charset=\"UTF-8\"><title>Search Result</title></head>\n"
        << "<body>\n"
        << "<center>\n"
        << "<h1 style=\"color:blue\">Search Server</h1>\n"
        << "</center>\n"
        << "<p  style=\"color:blue\">Result:<p>\n"
        << "<ul>\n";
    if (DBRes.size() > 0) {
        if (DBRes.size() > 10) {
            for (auto i = 0; i < 10; i++) {
                beast::ostream(res.body()) << "<li><a href=\"" << DBRes[i].first << "\">" << DBRes[i].first << "</a></li>";
            }
        }
        else {
        for (const auto& url : DBRes) {
            beast::ostream(res.body()) << "<li><a href=\"" << url.first << "\">" << url.first << "</a></li>";
        }
    }
    }
    else {
        beast::ostream(res.body()) << "<p>"
            << "No results found"
            << "</p>";
    }
    beast::ostream(res.body()) << "</ul>\n"
        << "</body>\n"
        << "</html>\n";
}

vector<pair<string, int>> Server::SearchDB(vector<string> str){
    host_ = IniParser_->get_data("host");
    port_ = IniParser_->get_data("port");
    dbname_ = IniParser_->get_data("dbname");
    user_ = IniParser_->get_data("user");
    password_ = IniParser_->get_data("password");
    try {
        pqxx::connection c("host = " + host_ + " port = " + port_ + " dbname = " + dbname_ + " user = " + user_ + " password = " + password_);
        pqxx::work tx(c);
        map<string, int> findURL;
        vector<pair<string, int>> sortedURL;
        if (str.size() > 0) {
            for (const auto& s : str) {
                for (auto [url, frequency] : tx.query<string, int>(
                    "SELECT DISTINCT documents.url, words.frequency FROM words INNER JOIN words_from_documents ON words_from_documents.id_words = words.id "
                    "INNER JOIN documents ON documents.id = words_from_documents.id_documents WHERE words.word = '" + s + "' GROUP BY documents.url, words.frequency;"))
                {
                    map<string, int>::iterator it = findURL.find(url);
                    if (it == findURL.end()) {
                        findURL[url] = frequency;
                    }
                    else {
                        findURL[url] += frequency;
                    }
                }
            }

            for (auto pair : findURL) {
                sortedURL.push_back(pair);
            }
            sort(sortedURL.begin(), sortedURL.end(), [](pair<string, int>& a, pair<string, int>& b) { return a.second > b.second; });
        }
        tx.commit();
        return sortedURL;
    }
    catch (pqxx::sql_error e) {
        cout << "SQL error" << e.what() << "\n";
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
    }
}


void Server::CheckDeadline() {
    auto self = shared_from_this();

    deadline_.async_wait(
        [self](beast::error_code ec)
        {
            if (!ec)
            {
                self->socket_.close(ec);
            }
        });
};
