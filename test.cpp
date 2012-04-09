#include <stdio.h>
#include <stdlib.h>

#include <postgresql/libpq-fe.h>

int main()
{
    const char  *conninfo;
    PGconn      *conn;
    PGresult    *res;

    conninfo = "dbname=db_exam_test host=127.0.0.1 user=postgres password=dbpost";

    conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK)
    {
        perror("connect fail");
    }

    res = PQexec(conn, "select * from users");

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "%s\n", PQresStatus(PQresultStatus(res)));
        exit(1);
    }
    printf("exec OK\n");

    unsigned int rows = PQntuples(res);
    unsigned int cols = PQnfields(res);

    for (unsigned int i = 0; i < cols; ++i)
    {
        fprintf(stdout, "%s\t", PQfname(res, i));
    }

    fprintf(stdout, "\n");

    for (unsigned int i = 0; i < rows; ++i)
    {
        for (unsigned int j = 0; j < cols; ++j)
        {
            fprintf(stdout, "%s\t", PQgetvalue(res, i, j));
        }
        fprintf(stdout, "\n");
    }
    
    PQclear(res);

    res = PQexec(conn, "SELECT email FROM address WHERE uid = (\
          SELECT id FROM users WHERE name = 'bell')");
    rows = PQntuples(res);
    cols = PQnfields(res);
    
    fprintf(stdout, "%s\n", PQfname(res, 0));
    fprintf(stdout, "%s\n", PQgetvalue(res, 0, 0));


    PQclear(res);
    return 0;
}
