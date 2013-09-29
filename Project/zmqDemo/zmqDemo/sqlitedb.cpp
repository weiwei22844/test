/****************************************************************************
**

** If you have any questions , please contact me
**
****************************************************************************/
#include "stdafx.h"
#include "sqlitedb.h"

#pragma comment(lib,"sqlite3/sqlite3.lib")

#define DATABASE_NAME "Bell.db3"

typedef struct _selfCbStruct{
    int (*pCallBack)(void*,int,char**,char**);
    void* pUserData;
    void* pSelfData;
}selfCbStruct;

SqliteDB::SqliteDB()
{
    m_dbName=DATABASE_NAME;
}

/****************************************************************************
* pUserData: param of user pass
* nColumn: 该条记录有多少个字段（记录有多少列）
* columnValue: 一维数组，每个元素表示一个字段内容（字符串，以\0结尾)
* columnName: 与columnValue对应，表示这个字段的字段名称
****************************************************************************/
static int ifTableExit(void *pUserData, int nColumn, char **columnValue, char **columnName){
    int i;
    char temp[200];
    int result;
    for(i=0; i<nColumn; i++){
        sprintf(temp, "%s = %s\n", columnName[i], columnValue[i] ? columnValue[i] : "NULL");
        OutputDebugString(temp);
        result = atoi(columnValue[i]);
        if(result == 0)
            *(BOOL*)pUserData = FALSE;
        else
            *(BOOL*)pUserData = TRUE;
    }
    return 0;
}

static int getCount(void *pUserData, int nColumn, char **columnValue, char **columnName){
    int result;
    if(nColumn != 1){
        result = -1;
    }else{
        result = atoi(columnValue[0]);
        *(int*)pUserData = result;
    }
    
    return 0;
}

static int selectCb(void *pData, int nColumn, char **columnValue, char **columnName){
    int result;
    selfCbStruct *pCbStruct = (selfCbStruct *)pData;
    *(int *)(pCbStruct->pSelfData) += 1;
    return pCbStruct->pCallBack(pCbStruct->pUserData, nColumn, columnValue, columnName);
}

static int deleteCb(void *pUserData, int nColumn, char **columnValue, char **columnName){
    if(nColumn > 0){
        *(int*)pUserData = nColumn;
    }else{
        *(int*)pUserData = 0;
    }
    return 1;
}

int SqliteDB::InitDataBase(CString dbName)
{
    m_dbName = dbName;

    char *zErrMsg = 0;
    int rt;
    char sqlstatement[500] = { 0 };
    BOOL bTableExit;

    rt = sqlite3_open(m_dbName, &m_pDataBase);
    if( rt ){
        sprintf(m_errorInfo, "Can't open database: %s", sqlite3_errmsg(m_pDataBase));
        return -1;
    }

    bTableExit = FALSE;
    sprintf(sqlstatement, "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='contact'");
    rt = sqlite3_exec(m_pDataBase, sqlstatement, ifTableExit, &bTableExit, &zErrMsg);
    if(rt != SQLITE_OK){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(m_pDataBase));
        sqlite3_close(m_pDataBase);
        return -2;
    }
    if(!bTableExit){
        rt = sqlite3_exec(m_pDataBase, "CREATE TABLE contact([Id] integer primary key,[UserName] varchar(32),[SipAddr] varchar(50),[Subscribe] int,[SubscribePolicy] int,[Status] int,[PictureId] varchar(50))", 0, 0, &zErrMsg);
        if(rt != SQLITE_OK) {
            sprintf(m_errorInfo, "create table contact failed: %s", sqlite3_errmsg(m_pDataBase));
            sqlite3_close(m_pDataBase);
            return -3;
        }

#if 1
        for(int i = 0; i < 33; i++){
            if(i%2)
                sprintf(sqlstatement, "insert into contact([UserName],[SipAddr],[Subscribe],[SubscribePolicy],[Status],[PictureId]) values('%d','sip:%d@192.168.1.112','%d',%d,%d,'%d.png');", i+100, i+100, i%2,i%6 == 0,i%2,i%5);
            else
                sprintf(sqlstatement, "insert into contact([UserName],[SipAddr],[Subscribe],[SubscribePolicy],[Status],[PictureId]) values('%d','sip:%d@192.168.1.112','%d',%d,%d,'%d.png');", i+100, i+100, i%3,i%7 == 0,i%2,i%5);
            rt=sqlite3_exec(m_pDataBase, sqlstatement, 0, 0, &zErrMsg);
            if(rt != SQLITE_OK) {
                sprintf(m_errorInfo, "insert table contact failed: %s", sqlite3_errmsg(m_pDataBase));
                sqlite3_close(m_pDataBase);
                return -4;
            }
        }
#endif
    }

    bTableExit = FALSE;
    sprintf(sqlstatement, "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='callLog'");
    rt = sqlite3_exec(m_pDataBase, sqlstatement, ifTableExit, &bTableExit, &zErrMsg);
    if(rt != SQLITE_OK){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(m_pDataBase));
        sqlite3_close(m_pDataBase);
        return -2;
    }
    
    if(!bTableExit){
        rt = sqlite3_exec(m_pDataBase, "CREATE TABLE callLog([Id] integer primary key,[Callin] int,[Disconnect] int,[Caller] varchar(50),[Answer] varchar(50),[StartDate] varchar(20),[Duration] int,[Quality] int)", 0, 0, &zErrMsg);
        if(rt != SQLITE_OK) {
            sprintf(m_errorInfo, "create table callLog failed: %s", sqlite3_errmsg(m_pDataBase));
            sqlite3_close(m_pDataBase);
            return -3;
        }
#if 1
        for(int i = 0; i < 100; i++){
            if(i%2)
                sprintf(sqlstatement, "insert into callLog([Callin],[Disconnect],[Caller],[Answer],[StartDate],[Duration],[Quality]) values('%d','%d','sip:%d@192.168.1.112','30044','20120824 %02d1630',%d,%d);", i%2, i%3 == 0, i+100, i%24, i+20,i%6 == 0);
            else
                sprintf(sqlstatement, "insert into callLog([Callin],[Disconnect],[Caller],[Answer],[StartDate],[Duration],[Quality]) values('%d','%d','sip:%d@192.168.1.112','30388','20130824 %02d1630',%d,%d);", i%3 == 0, i%3 !=0, i+100, i%24, i+20,i%7 == 0);
            rt=sqlite3_exec(m_pDataBase, sqlstatement, 0, 0, &zErrMsg);
            if(rt != SQLITE_OK) {
                sprintf(m_errorInfo, "insert table callLog failed: %s", sqlite3_errmsg(m_pDataBase));
                sqlite3_close(m_pDataBase);
                return -4;
            }
        }
#endif
    }

    sqlite3_close(m_pDataBase);

    return 1;
}

CString SqliteDB::getDbName()
{
    return m_dbName;
}

int SqliteDB::insertNewContact(const char* pName, const char* pSipAddr, int subscribe, int SubscribePolicy, int Status, const char *pImage)
{
    int rt;
    char *zErrMsg = 0;
    rt = sqlite3_open(m_dbName, &m_pDataBase);
    if( rt ){
        sprintf(m_errorInfo, "Can't open database: %s", sqlite3_errmsg(m_pDataBase));
        return -1;
    }
   
    char sqlstatement[500] = { 0 };
    sprintf(sqlstatement, "insert into contact([UserName],[SipAddr],[Subscribe],[SubscribePolicy],[Status],[PictureId]) values('%s','%s',%d,%d,%d,'%s');", pName, pSipAddr, subscribe, SubscribePolicy, Status, pImage);
    rt=sqlite3_exec(m_pDataBase, sqlstatement, 0, 0, &zErrMsg);
    if(rt != SQLITE_OK) {
        sprintf(m_errorInfo, "insert table contact failed: %s", sqlite3_errmsg(m_pDataBase));
        sqlite3_close(m_pDataBase);
        return -4;
    }
    sqlite3_close(m_pDataBase);
    return 1;
}

int SqliteDB::getContactCount()
{
    int rt;
    char *zErrMsg = 0;
    rt = sqlite3_open(m_dbName, &m_pDataBase);
    if( rt ){
        sprintf(m_errorInfo, "Can't open database: %s", sqlite3_errmsg(m_pDataBase));
        return -1;
    }

    int count;
    char sqlstatement[500] = { 0 };
    sprintf(sqlstatement, "SELECT count(*) FROM contact");
    rt = sqlite3_exec(m_pDataBase, sqlstatement, getCount, &count, &zErrMsg);
    if(rt != SQLITE_OK){
        sprintf(m_errorInfo, "insert table contact failed: %s", sqlite3_errmsg(m_pDataBase));
        sqlite3_close(m_pDataBase);
        return -3;
    }
    sqlite3_close(m_pDataBase);
    return count;
}

int SqliteDB::getCallLogCount()
{
    int rt;
    char *zErrMsg = 0;
    rt = sqlite3_open(m_dbName, &m_pDataBase);
    if( rt ){
        sprintf(m_errorInfo, "Can't open database: %s", sqlite3_errmsg(m_pDataBase));
        return -1;
    }

    int count;
    char sqlstatement[500] = { 0 };
    sprintf(sqlstatement, "SELECT count(*) FROM callLog");
    rt = sqlite3_exec(m_pDataBase, sqlstatement, getCount, &count, &zErrMsg);
    if(rt != SQLITE_OK){
        sprintf(m_errorInfo, "insert table callLog failed: %s", sqlite3_errmsg(m_pDataBase));
        sqlite3_close(m_pDataBase);
        return -3;
    }
    sqlite3_close(m_pDataBase);
    return count;
}

int SqliteDB::getContact(int offset, int size, int (*callback)(void*,int,char**,char**), void* pUserData)
{
    int rt;
    char *zErrMsg = 0;
    rt = sqlite3_open(m_dbName, &m_pDataBase);
    if( rt ){
        sprintf(m_errorInfo, "Can't open database: %s", sqlite3_errmsg(m_pDataBase));
        return -1;
    }

    int count = 0;
    selfCbStruct cbStruct;
    cbStruct.pCallBack = callback;
    cbStruct.pUserData = pUserData;
    cbStruct.pSelfData = &count;
    char sqlstatement[500] = { 0 };
    sprintf(sqlstatement, "SELECT * FROM contact limit %d offset %d;", size, offset);
    rt = sqlite3_exec(m_pDataBase, sqlstatement, selectCb, &cbStruct, &zErrMsg);
    if(rt != SQLITE_OK){
        sprintf(m_errorInfo, "insert table contact failed: %s", sqlite3_errmsg(m_pDataBase));
        sqlite3_close(m_pDataBase);
        return -3;
    }
    sqlite3_close(m_pDataBase);
    return count;
}

int SqliteDB::getCalllog(int offset, int size, int (*callback)(void*,int,char**,char**), void* pUserData)
{
    int rt;
    char *zErrMsg = 0;
    rt = sqlite3_open(m_dbName, &m_pDataBase);
    if( rt ){
        sprintf(m_errorInfo, "Can't open database: %s", sqlite3_errmsg(m_pDataBase));
        return -1;
    }

    int count = 0;
    selfCbStruct cbStruct;
    cbStruct.pCallBack = callback;
    cbStruct.pUserData = pUserData;
    cbStruct.pSelfData = &count;
    char sqlstatement[500] = { 0 };
    sprintf(sqlstatement, "SELECT * FROM callLog limit %d offset %d;", size, offset);
    rt = sqlite3_exec(m_pDataBase, sqlstatement, selectCb, &cbStruct, &zErrMsg);
    if(rt != SQLITE_OK){
        sprintf(m_errorInfo, "insert table callLog failed: %s", sqlite3_errmsg(m_pDataBase));
        sqlite3_close(m_pDataBase);
        return -3;
    }
    sqlite3_close(m_pDataBase);
    return count;
}

int SqliteDB::delContact(int index)
{
    int rt;
    char *zErrMsg = 0;
    rt = sqlite3_open(m_dbName, &m_pDataBase);
    if( rt ){
        sprintf(m_errorInfo, "Can't open database: %s", sqlite3_errmsg(m_pDataBase));
        return -1;
    }

    int nDeleteCnt;
    char sqlstatement[500] = { 0 };
    sprintf(sqlstatement, "DELETE FROM contact WHERE Id=%d;", index);
    rt = sqlite3_exec(m_pDataBase, sqlstatement, deleteCb, &nDeleteCnt, &zErrMsg);
    if(rt != SQLITE_OK){
        sprintf(m_errorInfo, "delete table contact failed: %s", sqlite3_errmsg(m_pDataBase));
        sqlite3_close(m_pDataBase);
        return -1;
    }
    sqlite3_close(m_pDataBase);
    return 1;
}

int SqliteDB::deleteCalllog(int index)
{
    int rt;
    char *zErrMsg = 0;
    rt = sqlite3_open(m_dbName, &m_pDataBase);
    if( rt ){
        sprintf(m_errorInfo, "Can't open database: %s", sqlite3_errmsg(m_pDataBase));
        return -1;
    }

    int nDeleteCnt;
    char sqlstatement[500] = { 0 };
    sprintf(sqlstatement, "DELETE FROM callLog WHERE Id=%d;", index);
    rt = sqlite3_exec(m_pDataBase, sqlstatement, deleteCb, &nDeleteCnt, &zErrMsg);
    if(rt != SQLITE_OK){
        sprintf(m_errorInfo, "delete table callLog failed: %s", sqlite3_errmsg(m_pDataBase));
        sqlite3_close(m_pDataBase);
        return -1;
    }
    sqlite3_close(m_pDataBase);
    return 1;
}

char* SqliteDB::getlastErrorInfo()
{
    return m_errorInfo;
}