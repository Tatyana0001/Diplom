#include <iostream>
#include <cstdlib>
#include <string>
#include <pqxx/pqxx>
#include "Database.h"
#include "IniParser.h"
#include "Spider.h"

using namespace std;
using namespace pqxx;

int main()
{
    IniParser IniParser("../../../../Spider/data.ini");
    Database database(IniParser);
    string startURL = IniParser.get_data("startpage");
    int depth = atoi(IniParser.get_data("depth").c_str());
    string port = IniParser.get_data("searchport");
    Spider Spider(startURL, depth, port);
    Spider.startSpider();
}