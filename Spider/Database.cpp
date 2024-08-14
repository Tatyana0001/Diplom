#include "Database.h"
#include <iostream>

using namespace std;

Database::Database(IniParser& IniParser) {
    if (!c) {
        try {
           // cout << IniParser.get_data("host") << "\n";
            c = new pqxx::connection("host = " + IniParser.get_data("host") + " port = " + IniParser.get_data("port") + " dbname = " + IniParser.get_data("dbname") + " user = " + IniParser.get_data("user") + " password = " + IniParser.get_data("password") + " ");
            pqxx::work tx(*c);
            tx.exec("CREATE TABLE IF NOT EXISTS documents (id SERIAL PRIMARY KEY, url VARCHAR(250) NOT NULL UNIQUE);");

            tx.exec("CREATE TABLE IF NOT EXISTS words (id SERIAL PRIMARY KEY, word VARCHAR(100) NOT NULL UNIQUE, frequency INTEGER NOT NULL);");

            tx.exec("CREATE TABLE IF NOT EXISTS words_from_documents ("
                "id SERIAL PRIMARY KEY, "
                "documents INTEGER NOT NULL REFERENCES documents(id), "
                "words INTEGER NOT NULL REFERENCES words(id))");
            tx.commit();
        }
        catch (pqxx::sql_error e) {
            cout << "SQL error" << e.what() << "\n";
        }
        catch (const std::exception ex) {
            cout << "General error" << ex.what() << "\n";
        }
    } 
}
    

