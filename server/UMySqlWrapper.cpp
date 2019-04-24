#include "mysql_connection.h"	
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include <cstring>

#include "UMySqlWrapper.h"

using namespace std;
//------------------------------------------------------------------------------
TMySqlWrapper::TMySqlWrapper()
{
} 
//------------------------------------------------------------------------------
TMySqlWrapper::~TMySqlWrapper()
{
    delete Res;
    delete PrepStmt;
    delete Stmt;
    delete Con;
    Con=NULL;
    Stmt=NULL;
    PrepStmt=NULL;
    Res=NULL;
}
//------------------------------------------------------------------------------
void TMySqlWrapper::Init(string url, string u, string p)
{
    HostPort=url;
    User=u;
    Password=p;
    Driver=NULL;
    Con=NULL;
    Stmt=NULL;
    PrepStmt=NULL;
    Res=NULL;
}
//------------------------------------------------------------------------------
string TMySqlWrapper::GetUrl()
{
    return HostPort;
}
//------------------------------------------------------------------------------
string TMySqlWrapper::GetUser()
{
    return User;
}
//------------------------------------------------------------------------------
string TMySqlWrapper::GetPassword()
{
    return Password;
}
//------------------------------------------------------------------------------
void TMySqlWrapper::ManageException(sql::SQLException &e,  const char *file, const char *function, long int line)
{
    if (e.getErrorCode() != 0)
    {
        memset(BufferError, 0, sizeof(BufferError));
        sprintf(BufferError, "SQLException file=%s, function=%s(), line=%ld, message=%s, code= %d, state=%s",
                            file, function, line,
                            e.what(), e.getErrorCode(), e.getSQLState().c_str());
    }
}
//------------------------------------------------------------------------------
bool TMySqlWrapper::Connect()
{
    bool rslt=false;
    try
    {
        Driver = get_driver_instance();
        Con = Driver->connect(HostPort, User, Password);
        bool myTrue = true;  
        Con->setClientOption("OPT_RECONNECT", &myTrue); 
        rslt=true;
    } 
    catch(sql::SQLException &e)
    {
        ManageException(e, __FILE__, __FUNCTION__, __LINE__);
    }
    return rslt;
}
//------------------------------------------------------------------------------
bool TMySqlWrapper::SwitchDb(const string& db_name)
{
    bool rslt=false;
    try
    {
        Con->setSchema(db_name);
        Stmt = Con->createStatement();
        rslt=true;
    }
    catch(sql::SQLException &e) 
    {
        ManageException(e, __FILE__, __FUNCTION__, __LINE__);
    }
    return rslt;
}
//------------------------------------------------------------------------------
bool TMySqlWrapper::Prepare(const string& query)
{
    bool rslt=false;
    try
    {
        PrepStmt = Con->prepareStatement(query);
        rslt=true;
    } 
    catch(sql::SQLException &e)
    {
        ManageException(e, __FILE__, __FUNCTION__, __LINE__);
    }
    return rslt;
}
//------------------------------------------------------------------------------
void TMySqlWrapper::DeletePrepare()
{
    delete PrepStmt;
    PrepStmt=NULL;
}
//------------------------------------------------------------------------------
void TMySqlWrapper::SetInt(const int& num, const int& data)
{
    PrepStmt->setInt(num, data);
}
//------------------------------------------------------------------------------
void TMySqlWrapper::SetULongInt(const int &num, const uint64_t &data)
{
    PrepStmt->setUInt64(num, data);
}
//------------------------------------------------------------------------------
void TMySqlWrapper::SetString(const int& num, const string& data)
{
    PrepStmt->setString(num, data);
}
//------------------------------------------------------------------------------
bool TMySqlWrapper::ExecuteQuery(const string& query)
{
    bool rslt=false;
    try
    {
        if(query != ""){
            Res = Stmt->executeQuery(query);
        } 
        else{
            Res = PrepStmt->executeQuery();
        }
        rslt=true;
    } 
    catch(sql::SQLException &e)
    {
        ManageException(e, __FILE__, __FUNCTION__, __LINE__);
    }
    return rslt;
}
//------------------------------------------------------------------------------
bool TMySqlWrapper::ExecuteUpdate(const string& act)
{
    bool rslt=false;
    try 
    {
        if(act != ""){
            Stmt->execute(act);
        }
        else{
            PrepStmt->execute();
        }
        rslt=true;
    } 
    catch(sql::SQLException &e)
    {
        ManageException(e, __FILE__, __FUNCTION__, __LINE__);
    }
    return rslt;
}
//------------------------------------------------------------------------------
void TMySqlWrapper::CloseCon()
{
    Con->close();
}
//------------------------------------------------------------------------------
bool TMySqlWrapper::Fetch()
{
    return Res->next();
}
//------------------------------------------------------------------------------
string TMySqlWrapper::GetString(const string& field)
{
    return Res->getString(field);
}
//------------------------------------------------------------------------------
string TMySqlWrapper::GetString(const int& index)
{
    return Res->getString(index);
}
//------------------------------------------------------------------------------
int TMySqlWrapper::GetInt(const string& field)
{
    return Res->getInt(field);
}
//------------------------------------------------------------------------------
int TMySqlWrapper::GetInt(const int& index)
{
    return Res->getInt(index);
}
//------------------------------------------------------------------------------
unsigned long int TMySqlWrapper::GetULongInt(const string &field)
{
    return Res->getUInt64(field);
}
//------------------------------------------------------------------------------
unsigned long int TMySqlWrapper::GetULongInt(const int &index)
{
    return Res->getUInt64(index);
}
//------------------------------------------------------------------------------
sql::ResultSet* TMySqlWrapper::GetRes()
{
    return Res;
}
