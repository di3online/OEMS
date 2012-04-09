#include "db.h"

#include <iostream>

const char *DB::conninfo = "host=127.0.0.1 user=postgres dbname=oes_db password=dbpost";

PGconn *DB::conn = NULL;

unsigned int DB::count = 0;


DB::DB()
{
    if (NULL == conn)
    {
        std::cerr << "New DB Connection has been established" << std::endl;
        conn = PQconnectdb(conninfo);
    } else if (PQstatus(conn) != CONNECTION_OK){
        PQfinish(conn);

        std::cerr << "New DB Connection has been established" << std::endl;
        conn = PQconnectdb(conninfo);
    }

    count++;
}

PGconn *DB::getConn()
{
    return conn;
}

bool DB::isConnected()
{
    return (PQstatus(conn) == CONNECTION_OK);
}

void DB::closeConn()
{
    count--;

    if (0 == count)
    {
        std::cerr << "DBConnection has been closed" << std::endl;
        PQfinish(conn);
        conn = NULL;
    }
    return ;
}
