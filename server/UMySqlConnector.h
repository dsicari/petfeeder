#ifndef MYSQLCONNECTOR_H
#define MYSQLCONNECTOR_H

// apt-get install libmysqlcppconn-dev, compilar com -lmysqlcppconn
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include <stdio.h>
#include <stdlib.h>
#include <cstring> 
#include <string>

using namespace std;

class TMySqlConnector
{
private:
    sql::Driver *Driver;
    sql::Connection *Con;
    sql::Statement *Stmt;
    sql::ResultSet *Res;
    sql::PreparedStatement *PrepStmt;
    string Host;
    string Database;
    string User;
    string Password;
    
    void Exception2Buffer(sql::SQLException &e,  const char *file, const char *function, long int line);
    
public:
    char BufferError[2048];
    TMySqlConnector(const char *host, const char *database, const char *user, const char *password);
    ~TMySqlConnector();
    bool ConnectDB();
    bool Prepare(const string &query);
    void DeletePrepare();
    void SetInt(const int &num, const int &data);
    void SetULongInt(const int &num, const uint64_t &data);
    void SetString(const int &num, const string &data);
    bool ExecuteQuery(const string& query);
    bool ExecuteUpdate(const string& act);
    void CloseCon();
    bool Fetch();
    string GetString(const string& field);
    string GetString(const int& index);
    int GetInt(const string& field);
    int GetInt(const int& index);
    sql::ResultSet* GetRes();
};

#endif

