/**
 * @file choice.cpp
 * @brief
 * @author ZhuZY
 * @version 1.7
 * @date 2012-05-03
 */
#include "choice.h"

using namespace std;

/**
 * @brief createChoice
 *          Check the userID,
 *          and return the cid (the id of choice)
            and add a choice to DB
 * @param userID
 *          User's ID which will be checked
 *
 * @return
 *          cid
 *          if userID is permission
 */
cid_t createChoice(const uid_t &uid, int &err, PGconn *dbconn)
{
    cid_t cid;
    err = PC_UNKNOWNERROR;
    PGconn *conn = dbconn;


    gid_t gid = getGIDByUID(uid, err, conn);

    if ((gid != GID_ADMIN) && (gid != GID_TEACHER))
    {
        err = PC_NOPERMISSION;
        return "PC_NOPERMISSION";
    }

    if (PQstatus(conn) != CONNECTION_OK)
    {
        err = PC_DBERROR;
    }

    char sql[300];

    snprintf(sql, sizeof(sql), "INSERT INTO choice(question_id, Answer_Content) VALUES(NULL, NULL) RETURNING choice_id");

    PGresult *res;
    //Exec the SQL query

    res = PQexec(conn, sql);
    if ((PQresultStatus(res) != PGRES_TUPLES_OK))
    {
        err = PC_DBERROR;
        PQclear(res);
        return "";
    }

    cid = PQgetvalue(res, 0, 0);
    PQclear(res);
    err = PC_SUCCESSFUL;

    return cid;
}




/**
 * @brief setChoiceDescription
 *          Check the userID, cid
 *          then modify the description of the choice
 * @param userID
 *          User's ID will be checked
 * @param cid
 *          choice ID will be checked
 * @param description
 *          description will be modified in the DB
 * @return
 *          void
 */
void setChoiceDescription(const uid_t &uid, const cid_t &cid, string description, int &err, PGconn *dbconn)
{
    err = PC_UNKNOWNERROR;
    PGconn *conn = dbconn;

    gid_t gid = getGIDByUID(uid, err, conn);

    if ((gid != GID_ADMIN) && (gid != GID_TEACHER))
    {
        err = PC_NOPERMISSION;
        return;
    }

    if (PQstatus(conn) != CONNECTION_OK)
    {
        err = PC_DBERROR;
        return;
    }

    char sql[300];
    size_t number_cid;
    stringstream ss;

    ss << cid;
    ss >> number_cid;

    snprintf(sql, sizeof(sql), "SELECT * FROM choice WHERE choice_id = %lu", number_cid);

    //Exec the SQL query
    PGresult *res;
    res = PQexec(conn, sql);

    //check if the cid is exist
    if( PQntuples(res) == 0)
    {
        PQclear(res);
        err = PC_NOTFOUND;
        return;
    }

    snprintf(sql, sizeof(sql), "UPDATE choice SET answer_content = '%s' WHERE choice_id = %lu", description.c_str(), number_cid);
    res = PQexec(conn, sql);

    if ((PQresultStatus(res) != PGRES_COMMAND_OK))
    {
        PQclear(res);
        err = PC_SYSTEMERROR;
        return;
    }

    PQclear(res);
    err = PC_SUCCESSFUL;

    return;
}
