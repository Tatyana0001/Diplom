#include <iostream>
#include <cstdlib>
#include <string>
#include <pqxx/pqxx>
#include "Server.h"



using namespace std;
using namespace pqxx;

void httpServer(tcp::acceptor& acceptor, tcp::socket& socket, IniParser& IniParser) {
    acceptor.async_accept(socket, [&](beast::error_code ec) {
        if (!ec)
            make_shared<Server>(move(socket), &IniParser)->start();
        httpServer(acceptor, socket, IniParser);
        });
}


int main()
{
    map<string, string> DB;
    try {
        SetConsoleOutputCP(CP_UTF8);
        IniParser IniParser("../../../../Spider/data.ini");
        unsigned short port = static_cast<unsigned short>(atoi(IniParser.get_data("searchport").c_str()));
        auto const address = net::ip::make_address("0.0.0.0");
        net::io_context ioc{ 1 };
        tcp::acceptor acceptor{ ioc, { address, port } };
        tcp::socket socket{ ioc };
        httpServer(acceptor, socket, IniParser);
        cout << "Connect to http://localhost:" << port << " for Search Server" << "\n";

        ioc.run();
    }
    catch (exception const& e)
    {
        cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}