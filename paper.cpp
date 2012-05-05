/**
 * @file paper.cpp
 * @brief
 * @author WangXi
 * @version 1.2
 * @date 2012-05-4
 */

#include "paper.h"

using namespace std;

/**
 * @brief addQuestionToPaper
 *          Check cookie, and add a question to a paper.
 *
 * @param cookie
 *          User's cookie which can identify ones personal indentity
 *
 * @return
 *          The string which contain some
 *          information if the indentity is teacher or administrator
 */
void
addQuestionToPaper(const uid_t &uid, const qid_t &qid, const pid_t &pid,
	int &err, PGconn *dbconn)
{
    err = PC_UNKNOWNERROR;

    PGconn *conn = dbconn;
    string ret;

    if (PQstatus(conn) != CONNECTION_OK)
    {
        ret = "";
        err = PC_DBERROR;
        return ;
    }

    gid_t groupID = getGIDByUID(uid, err, conn);
    if (err != PC_SUCCESSFUL)
    {
        //err = PC_DBERROR;
        return ;
    }

    if(groupID == GID_ADMIN || groupID == GID_TEACHER)
    {
        char sql[300];

        char cpid[2 * pid.size() + 1];
        char cqid[2 * qid.size() + 1];

        PQescapeString(cpid, pid.c_str(), pid.size());
        PQescapeString(cqid, qid.c_str(), qid.size());

        snprintf(sql,sizeof(sql), "SELECT * FROM paper WHERE paper_id = '%s'",cpid);
        PGresult *res = PQexec(conn, sql);

        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            err = PC_NOTFOUND;
            PQclear(res);
            return ;
        }

        snprintf(sql,sizeof(sql), "SELECT * FROM question WHERE question_id ='%s'",cqid);
        res = PQexec(conn,sql);

        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            err = PC_NOTFOUND;
            PQclear(res);
            return ;
        }

        snprintf(sql, sizeof(sql), "UPDATE question SET paper_id = '%s' WHERE question_id ='%s'",cpid,cqid);
        res = PQexec(conn, sql);

        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {//If exection failed
            err = PC_DBERROR;
            PQclear(res);
            return ;
        }

        PQclear(res);

        err = PC_SUCCESSFUL;

        return ;
    }else
    {
        err = PC_NOPERMISSION;
        return ;
    }

}



/**
 * @brief createPaper
 *          Check cookie, and create a new paper in db.
 *
 * @param cookie
 *          User's cookie which can identify ones personal indentity
 *
 * @return
 *          The string which contain some
 *          information if the indentity is teacher or administrator
 */
pid_t createPaper(const uid_t &uid,
	int &err, PGconn *dbconn)
{
    err = PC_UNKNOWNERROR;
    PGconn *conn = dbconn;

    string ret = "";

    if (PQstatus(conn) != CONNECTION_OK)
    {
        err = PC_DBERROR;
        ret = "";
        return ret;
    }

    gid_t groupID = getGIDByUID (uid,err,dbconn);           //get user's group id
    if (err != PC_SUCCESSFUL)
    {
        ret = "";
        return ret;
    }
    if (groupID == GID_ADMIN || groupID == GID_TEACHER)              //check user's permission limit
    {
        char sql[300];
        snprintf(sql, sizeof(sql), "INSERT INTO paper VALUES (DEFAULT) RETURNING paper_id");

        PGresult *res = PQexec(conn, sql);

        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {//If exection failed
            err = PC_DBERROR;
            ret = "";
            PQclear(res);
            return ret;
        }

        string temp_id = "";
        temp_id = PQgetvalue(res,0,0);
        ret = temp_id;

        PQclear(res);

        //set the err
        err = PC_SUCCESSFUL;

        return ret;
    }else
    {
        err = PC_NOPERMISSION;
        return ret;
    }

}

/**
 * @brief deletePaper
 *          Check cookie, and delete a paper in db.
 *
 * @param cookie
 *          User's cookie which can identify ones personal indentity
 *
 * @return
 *          The string which contain some
 *          information if the indentity is teacher or administrator
 */
void deletePaper(const uid_t &uid, const pid_t &pid,
	int &err, PGconn *dbconn)
{
    err = PC_UNKNOWNERROR;
    PGconn *conn = dbconn;

    string ret = "";
    if (PQstatus(conn) != CONNECTION_OK)
    {
        //cout <<"++++";
        err = PC_DBERROR;
        return ;
    }

    gid_t groupID = getGIDByUID (uid,err,dbconn);
    if (err != PC_SUCCESSFUL)
    {
        return ;
    }

    if(groupID == GID_ADMIN || groupID == GID_TEACHER)
    {
        char cpid[2 *pid.size() + 1];

        PQescapeString(cpid, pid.c_str(), pid.size());

        char sql[300];

        snprintf(sql, sizeof(sql), "SELECT paper_id FROM paper where paper_id = '%s' ",cpid);
        PGresult *res = PQexec(conn, sql);
        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            err = PC_DBERROR;
            //cout <<"++++";
            PQclear(res);
            return ;
        }

        if (PQntuples(res) == 0)
        {//If return null
            err = PC_NOTFOUND;
            PQclear(res);
            return ;
        }

        PQclear(res);

        snprintf(sql, sizeof(sql), "DELETE FROM paper WHERE paper_id='%s'", cpid);
        //Exec the SQL query
        res = PQexec(conn, sql);

        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {//If exection failed
            err = PC_DBERROR;
            //cout <<"++++";
            PQclear(res);
            return ;
        }

        PQclear(res);
        err = PC_SUCCESSFUL;
        return ;
    }else
    {
        err = PC_NOPERMISSION;
        return ;
    }
}

