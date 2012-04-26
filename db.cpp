#include "db.h"

#include <iostream>

const char *DB::conninfo = "host=127.0.0.1 user=postgres dbname=oes_db password=dbpost";

struct DBConns *DB::p_dbconns = NULL;

DB::DB()
{
    this->conn = NULL;
    sem_wait(&p_dbconns->sem_conns);
    sem_wait(&p_dbconns->mutex);
    for (int i = 0; i < p_dbconns->count; ++i)
    {
        if (!p_dbconns->used[i])
        {
            this->conn = p_dbconns->conns[i];
            p_dbconns->used[i] = 1;
            break;
        }
    }

    sem_post(&p_dbconns->mutex);

}

DB::~DB()
{
    sem_wait(&p_dbconns->mutex);
    for (int i = 0; i < p_dbconns->count; ++i)
    {
        if (this->conn == p_dbconns->conns[i] 
                && p_dbconns->used[i] == 1)
        {
            p_dbconns->used[i] = 0;
            break;
        }
    }
    sem_post(&p_dbconns->sem_conns);
    sem_post(&p_dbconns->mutex);
}

PGconn *DB::getConn()
{
    return conn;
}

bool DB::isConnected()
{
    return (PQstatus(conn) == CONNECTION_OK);
}

void DB::closeConns()
{

    return ;
}
