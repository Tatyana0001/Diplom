#pragma once
#include <string>
#include <fstream>
#include <map>

using namespace std;

class IniParser {
public:
    IniParser();
    IniParser(const string& ini_file);
    string get_data(string name_data);
private:
    ifstream fin;
    map<string, string> initial_data;
    void Parse();
};
