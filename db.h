#ifndef __INCLUDE_DB_
#define __INCLUDE_DB_

#include <postgresql/libpq-fe.h>

#include <queue>
#include <vector>

#include <semaphore.h>
#define DBMAXCONNS 30

struct DBConns{
    sem_t mutex;
    sem_t sem_conns;
    int count;
    PGconn *conns[DBMAXCONNS];
    char used[DBMAXCONNS];
};
class DB{
public:
    
    static const unsigned int MAX_CON = DBMAXCONNS;
    static struct DBConns *p_dbconns;
    
    static const char *conninfo;
    DB();

    ~DB();
    PGconn *getConn();

    bool isConnected();

private:
    
    PGconn *conn;


    static void closeConns();
};

#endif //__INCLUDE_DB_
