#ifndef __INCLUDE_DB_
#define __INCLUDE_DB_

#include <postgresql/libpq-fe.h>

class DB{
public:
    
    DB();

    ~DB()
    {
        closeConn();
    }

    PGconn *getConn();

    bool isConnected();

private:

    static const char *conninfo;
    static PGconn *conn;
    static unsigned int count;

    static void closeConn();
};

#endif //__INCLUDE_DB_
