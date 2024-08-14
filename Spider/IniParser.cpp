#include <iostream>
#include "IniParser.h"

using namespace std;
IniParser::IniParser(){}
IniParser::IniParser(const string& ini_file) {
    fin.open(ini_file);
    if (!fin.is_open()) {
        throw runtime_error("File is not found");
    }
    else {
        Parse();
    }
}

string IniParser::get_data(string name_data) {
       // data.insert(initial_data.begin(), initial_data.end());
       // return data;
        //for (int i = 0; i < initial_data.end(); i++) {
        //    data[i] = initial_data[i];
        //}
    return initial_data[name_data];
}

void IniParser::Parse() {
    int line_num = 1;
    string line;
    while (!fin.eof() && getline(fin, line)) {
        if (line.length() > 0) {
            auto ravno = line.find("=");
            if (ravno > 0 && ravno < 100) {
                auto probel = line.find(" ");
                if (probel > 0 && probel < 100) {
                    line = line.substr(0, probel);
                }
                string name = line.substr(0, ravno);
                string value = line.substr((ravno + 1), line.length());
                initial_data[name] = value;
            }
        }
        ++line_num;
    }
    fin.close();
}
