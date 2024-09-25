#pragma once
#include "IniParser.h"
#include <map>
#include <pqxx/pqxx>


class Database {
private:
    bool connect();
    int sql(const string& str);
protected:
    string host_, port_, dbname_, user_, password_;
public:
    map<string, string> db_datas;
    Database(IniParser& Iniparser);
    Database() = default;
    vector<string> links_;
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    pqxx::connection* c{ nullptr };
    int create();
    void SaveDatasToDB(map<string, int> words, const string& link, map<string, string> db_datas);
    ~Database();
    int id_word = 1;
};
