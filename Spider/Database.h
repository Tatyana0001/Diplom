#pragma once
#include "IniParser.h"
#include <pqxx/pqxx>


class Database {
private:
 
public:
    Database(IniParser& IniParser);
    Database() = default;
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    pqxx::connection* c{ nullptr };
  
};
