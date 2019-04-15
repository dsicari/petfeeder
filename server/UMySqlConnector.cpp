
#include "UMySqlConnector.h"
//------------------------------------------------------------------------------
TMySqlConnector::TMySqlConnector(const char *host, const char *database, const char *user, const char *password)
{       
    Host=host;
    Database=database;
    User=user;
    Password=password;
}
//------------------------------------------------------------------------------
TMySqlConnector::~TMySqlConnector() { 
    Con=NULL;
    Stmt=NULL; 
    PrepStmt=NULL;
    Res=NULL;    

    delete Con;
    delete Stmt;
    delete PrepStmt;
    delete Res;
}
//------------------------------------------------------------------------------
void TMySqlConnector::Exception2Buffer(sql::SQLException &e,  const char *file, const char *function, long int line)
{
    memset(BufferError, 0, sizeof(BufferError));
    sprintf(BufferError, "SQLException file=%s, function=%s(), line=%ld, message=%s, code= %d, state=%s",
                            file, function, line,
                            e.what(), e.getErrorCode(), e.getSQLState().c_str()); 
}
//------------------------------------------------------------------------------
bool TMySqlConnector::ConnectDB()
{
    bool rslt=false;
    try
    {    
        Driver = get_driver_instance();
        Con = Driver->connect(Host, User, Password);
        bool conTrue=true;
        Con->setClientOption("OPT_RECONNECT", &conTrue); 
        Con->setSchema(Database);
        rslt=true;
    }
    catch (sql::SQLException &e) {
        Exception2Buffer(e, __FILE__, __FUNCTION__, __LINE__);
    } 
    return rslt;
}
//------------------------------------------------------------------------------
bool TMySqlConnector::Prepare(const string &query)
{
    bool rslt=false;
    try
    {
        PrepStmt = Con->prepareStatement(query);
        rslt=true;
    } 
    catch (sql::SQLException &e){
        Exception2Buffer(e, __FILE__, __FUNCTION__, __LINE__);
    }
    return rslt;
}
//------------------------------------------------------------------------------
void TMySqlConnector::DeletePrepare()
{
    PrepStmt=NULL;
    delete PrepStmt;	
}
//------------------------------------------------------------------------------
void TMySqlConnector::SetInt(const int &num, const int &data)
{
    PrepStmt->setInt(num, data);
}
//------------------------------------------------------------------------------
void TMySqlConnector::SetULongInt(const int &num, const uint64_t &data)
{
    PrepStmt->setUInt64(num, data);
}
//------------------------------------------------------------------------------
void TMySqlConnector::SetString(const int &num, const string &data)
{
    PrepStmt->setString(num, data);
}
//------------------------------------------------------------------------------
bool TMySqlConnector::ExecuteQuery(const string& query)
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
    catch (sql::SQLException &e) {
        Exception2Buffer(e, __FILE__, __FUNCTION__, __LINE__);
    }
    return rslt;
}
//------------------------------------------------------------------------------
bool TMySqlConnector::ExecuteUpdate(const string& act)
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
    catch(sql::SQLException &e){
        Exception2Buffer(e, __FILE__, __FUNCTION__, __LINE__);
    }
    return rslt;
}
//------------------------------------------------------------------------------
void TMySqlConnector::CloseCon()
{
    Con->close();
}
//------------------------------------------------------------------------------
bool TMySqlConnector::Fetch()
{
    return Res->next();
}
//------------------------------------------------------------------------------
string TMySqlConnector::GetString(const string& field)
{
    return Res->getString(field);
}
//------------------------------------------------------------------------------
string TMySqlConnector::GetString(const int& index)
{
    return Res->getString(index);
}
//------------------------------------------------------------------------------
int TMySqlConnector::GetInt(const string& field)
{
    return Res->getInt(field);
}
//------------------------------------------------------------------------------
int TMySqlConnector::GetInt(const int& index)
{
    return Res->getInt(index);
}
//------------------------------------------------------------------------------
sql::ResultSet* TMySqlConnector::GetRes()
{
    return Res;
}

