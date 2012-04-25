#include "db.h"

#include <iostream>

const char *DB::conninfo = "host=127.0.0.1 user=postgres dbname=oes_db password=dbpost";

DB::DB()
{
    std::cerr << "New DB Connection has been established" << std::endl;
    conn = PQconnectdb(conninfo);

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
    std::cerr << "DBConnection has been closed" << std::endl;
    PQfinish(conn);
    conn = NULL;
    return ;
}
