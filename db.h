#ifndef __INCLUDE_DB_
#define __INCLUDE_DB_

#include <postgresql/libpq-fe.h>

#include <queue>
#include <vector>

#include <semaphore.h>
#define DBMAXCONNS 50

//init in init_db() of mainloop.cpp
struct DBConns{
    
    //Allow only one can change the following data
    sem_t mutex;
    //Set how many connections can be used
    sem_t sem_conns;

    //Number of connections
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
