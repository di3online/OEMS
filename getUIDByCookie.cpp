/**
 * @file getCIDByCookie.cpp
 * @brief
 * @author MoShuqi
 * @version 1.0
 * @date 2012-04-12
 */
#include "getUIDByCookie.h"


uid_t getUIDByCookie(const string &cookie, int &err, PGconn *dbconn)
{

    err = PC_UNKNOWNERROR;
    PGconn *conn = dbconn;
    uid_t userID;

    if (PQstatus(conn) != CONNECTION_OK)
    {
        err = PC_DBERROR;
        return userID;
    }

    if (userID.size() > MAXLEN_COOKIESIZE)
    {
        err = PC_INPUTFORMATERROR;
        return userID;
    }

    //escapte string of cookie
    char ccook[2 * cookie.size() + 1];
    PQescapeString(ccook, cookie.c_str(), cookie.size());

    string t = ccook;

    string sql = "SELECT user_id FROM users WHERE cookie ='"+t+"'";

    //Exec the SQL query
    PGresult *res = NULL;
    res = PQexec(conn, sql.c_str());

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {//If exection failed
        err = PC_DBERROR;
        PQclear(res);
        return userID;
    }

    if (PQntuples(res) == 0)
    {//If userID doesn't match password
        err = PC_MISTATCH;
        PQclear(res);
        return userID;
    }

    userID = PQgetvalue(res, 0, 0);

    //Updated by: Lai
    err = PC_SUCCESSFUL;

    PQclear(res);

    return userID;
}
