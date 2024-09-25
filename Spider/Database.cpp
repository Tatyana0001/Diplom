#include "Database.h"
#include <iostream>

using namespace std;

Database::Database(IniParser& IniParser) {
    host_ = IniParser.get_data("host");
    port_ = IniParser.get_data("port");
    dbname_ = IniParser.get_data("dbname");
    user_ = IniParser.get_data("user");
    password_ = IniParser.get_data("password");
}
   
bool Database::connect() {
    if (!c) {
        try {
            c = new pqxx::connection("host = " + host_ + " port = " + port_ + " dbname = " + dbname_ + " user = " + user_ + " password = " + password_);// +" ");
        }
        catch (pqxx::sql_error e) {
            cout << "SQL error" << e.what() << "\n";
        }
        catch (const std::exception ex) {
            cout << "General error" << ex.what() << "\n";
        }
        
    } return c->is_open();
}

int Database::sql(const std::string& str) {
    if (!c || !c->is_open()) {
        if (!connect()) {
            std::cout << "Failed to connect to the database\n";
            return -1;
        }
    }
    pqxx::work tx(*c);
    try {
        pqxx::result result = tx.exec(str);
        tx.commit();
        return result.affected_rows();
    }
    catch (pqxx::sql_error e) {
        std::cout << "SQL error" << e.what() << "\n";
    }
    catch (const std::exception ex) {
        std::cout << "General error" << ex.what() << "\n";
    }
}
    
int Database::create() {
    return sql("CREATE TABLE IF NOT EXISTS documents (id SERIAL PRIMARY KEY UNIQUE, url VARCHAR(250) NOT NULL UNIQUE);"
        "CREATE TABLE IF NOT EXISTS words (id SERIAL PRIMARY KEY UNIQUE, word VARCHAR(100) NOT NULL, frequency INTEGER NOT NULL);"
        "CREATE TABLE IF NOT EXISTS words_from_documents ("
        "id_documents INTEGER NOT NULL, "
        "id_words INTEGER NOT NULL, "
        "FOREIGN KEY(id_documents) REFERENCES documents(id), "
        "FOREIGN KEY(id_words) REFERENCES words(id));");
}


void Database::SaveDatasToDB(map<string, int> words, const string& link, map<string, string> db_datas) {
    try {
        if (host_ == "") {
            host_ = db_datas["host"];
            port_ = db_datas["port"];
            dbname_ = db_datas["dbname"];
            user_ = db_datas["user"];
            password_ = db_datas["password"];
        }
        sql("INSERT INTO documents(url) VALUES('" + link + "');");
        for (auto& i : words) {
            sql("INSERT INTO words(id, word, frequency) VALUES('" + to_string(id_word) + "', '" + i.first + "', '" + to_string(i.second) + "'); ");
            sql("INSERT INTO words_from_documents(id_documents, id_words) "
                "VALUES((SELECT id FROM documents WHERE url = '" + link + "'), '" + to_string(id_word) + "');");
            id_word++;
        }

    }
    catch (pqxx::sql_error e) {
        cout << "SQL error" << e.what() << "\n";
    }
    catch (const std::exception ex) {
        cout << "General error" << ex.what() << "\n";
    }
}

Database::~Database() {
    if (c) {
        c->close(); delete c;
    }
}

    

