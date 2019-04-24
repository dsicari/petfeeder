#ifndef UMYSQLWRAPPER_H
#define UMYSQLWRAPPER_H

#include "mysql_connection.h"
	
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

using namespace std;

class TMySqlWrapper
{
private:
    string HostPort;
    string User;
    string Password;
    sql::Driver* Driver;
    sql::Connection* Con;
    sql::Statement* Stmt;
    sql::PreparedStatement* PrepStmt;
    sql::ResultSet* Res;
        
public:
    TMySqlWrapper();
    ~TMySqlWrapper();
    void Init(string url, string u, string p);
    string GetUrl();
    string GetUser();
    string GetPassword();
    void ManageException(sql::SQLException &e,  const char *file, const char *function, long int line);
    bool Connect();
    bool SwitchDb(const string& db_name);
    bool Prepare(const string& query);
    void DeletePrepare();
    void SetInt(const int& num, const int& data);
    void SetULongInt(const int &num, const uint64_t &data);
    void SetString(const int& num, const string& data);
    bool ExecuteQuery(const string& query = "");
    bool ExecuteUpdate(const string& sql ="");
    bool Fetch();
    sql::ResultSet* GetRes();
    string GetString(const string& field);
    string GetString(const int& index);
    int GetInt(const string& field);
    int GetInt(const int& index);
    unsigned long int GetULongInt(const string &field);
    unsigned long int GetULongInt(const int &index);
    void CloseCon();
    
    char BufferError[2048];
};

#endif

