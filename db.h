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
    PGconn *conn;

    void closeConn();
};

#endif //__INCLUDE_DB_
