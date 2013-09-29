/****************************************************************************
**
** Date    : 2010-07-07
** Author  : furtherchan
** E-Mail  : cnsilan@163.com

** If you have any questions , please contact me
**
****************************************************************************/

#ifndef SQLITEDB_H
#define SQLITEDB_H
#include "sqlite3/sqlite3.h"

class SqliteDB
{
public:
    SqliteDB();

    int InitDataBase(CString dbName);
    
    //member function
    
    CString getDbName();
    int insertNewContact(const char* pName, const char* pSipAddr, int subscribe, int SubscribePolicy, int Status, const char *pImage);
    int getContactCount();
    int getCallLogCount();
    int getContact(int offset, int size, int (*callback)(void*,int,char**,char**), void* pUserData);
    int getCalllog(int offset, int size, int (*callback)(void*,int,char**,char**), void* pUserData);
    int delContact(int index);
    int deleteCalllog(int index);
    char* getlastErrorInfo();

private:
    sqlite3 *m_pDataBase;
    CString m_dbName;
    char m_errorInfo[100];
};

#endif // SQLITEDB_H
