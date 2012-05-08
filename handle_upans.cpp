/**
 * @file handle_upans.cpp
 * @brief 
 * @author LaiSHZH
 * @version 1.0
 * @date 2012-05-05
 */
#include <cstring>

#include "handlers.h"
#include "getUIDByCookie.h"
#include "getGIDByUID.h"
#include "score.h"

using namespace std;

string 
handle_UPANS(const string &rawtext)
{
    int err;
    string ret;
    size_t start = rawtext.find(' ');
    size_t end = rawtext.find(' ', start + 2);

    if (start == string::npos || end == string::npos)
    {
        err = PC_INPUTFORMATERROR;
        ret += sys_error(err);
        ret += "\r\n\r\n";
        return ret;
    }

    qid_t qid = rawtext.substr(start + 1, end - start -1);

    start = end + 1;
    end = rawtext.find_first_of(" \r\n", start + 1);

    if (end == string::npos)
    {
        err = PC_INPUTFORMATERROR;
        ret += sys_error(err);
        ret += "\r\n\r\n";
        return ret;
    }

    cid_t cid = rawtext.substr(start, end - start);

    const char *strfind = "Cookie: ";

    start = rawtext.find(strfind, end);
    if (start == string::npos)
    {
        err = PC_INPUTFORMATERROR;
        ret += sys_error(err);
        ret += "\r\n\r\n";
        return ret;
    }

    start += strlen(strfind);

    end = rawtext.find_first_of(" \r\n", start);

    if (end == string::npos)
    {
        err = PC_INPUTFORMATERROR;
        ret += sys_error(err);
        ret += "\r\n\r\n";
        return ret;
    }

    string cookie = rawtext.substr(start, end - start);

    DB db;
    PGconn *dbconn = db.getConn();

    uid_t uid = getUIDByCookie(cookie, err, dbconn);
    if (err != PC_SUCCESSFUL)
    {
        ret = sys_error(err);
        ret += "\r\n\r\n";
        return ret;
    }

    char cqid[qid.size() * 2 + 1];
    char ccid[qid.size() * 2 + 1];

    PQescapeString(cqid, qid.c_str(), qid.size());
    PQescapeString(ccid, cid.c_str(), cid.size());

    char sqlquery[MAXBUF];
    snprintf(sqlquery, sizeof(sqlquery), 
            "SELECT question_id FROM question "
            "WHERE paper_id = ( "
                "SELECT currentp FROM users "
                "WHERE user_id = '%s' "
            ") AND question_id = %s",
        uid.c_str(), cqid);
    
    PGresult *sqlres = PQexec(dbconn, sqlquery);

    if (PQresultStatus(sqlres) != PGRES_TUPLES_OK)
    {
        err = PC_DBERROR;
        ret = sys_error(err);
        ret += "\r\n\r\n";

        ret += PQerrorMessage(dbconn);
        PQclear(sqlres);
        return ret;
    }

    if (PQntuples(sqlres) == 0)
    {
        err = PC_CONDITIONERROR;
        ret = sys_error(err);
        ret += "\r\n\r\n";
        PQclear(sqlres);
        return ret;
    } 

    PQclear(sqlres);

    snprintf(sqlquery, sizeof(sqlquery), 
            "SELECT answer FROM answer_sheet "
            "WHERE user_id = '%s' AND question_id = '%s'", 
            uid.c_str(), cqid);

    sqlres = PQexec(dbconn, sqlquery);

    if (PQresultStatus(sqlres) != PGRES_TUPLES_OK)
    {
        err = PC_DBERROR;
        ret = sys_error(err);
        ret += "\r\n\r\n";

        ret += PQerrorMessage(dbconn);
        PQclear(sqlres);
        return ret;
    }

    if (PQntuples(sqlres) == 0)
    {
        PQclear(sqlres);
        
        snprintf(sqlquery, sizeof(sqlquery), 
                "INSERT INTO answer_sheet (user_id, question_id, answer, score)"
                " VALUES (%s, %s, %s, 0)", 
                uid.c_str(), cqid, ccid);

        sqlres = PQexec(dbconn, sqlquery);

    } else {
        PQclear(sqlres);

        snprintf(sqlquery, sizeof(sqlquery),
                "UPDATE answer_sheet SET answer = %s, score = 0 "
                "WHERE user_id = '%s' AND question_id = '%s'",
                ccid, uid.c_str(), cqid);

        sqlres = PQexec(dbconn, sqlquery);
    }

    if (PQresultStatus(sqlres) == PGRES_TUPLES_OK 
            || PQresultStatus(sqlres) == PGRES_COMMAND_OK)
    {
        err = PC_SUCCESSFUL;
        PQclear(sqlres);

//        snprintf(sqlquery, sizeof(sqlquery), 
//                "UPDATE answer_sheet SET score = ("
//                    "SELECT score FROM question "
//                    "WHERE question_id = '%s'"
//                ") WHERE user_id = '%s' AND question_id = '%s' "
//                    "AND answer = ("
//                        "SELECT key FROM question WHERE question_id = '%s'"
//                    ")",
//                    cqid, uid.c_str(), cqid, cqid);
//
        snprintf(sqlquery, sizeof(sqlquery),
                "UPDATE users SET qflag = ( "
                    "SELECT qflag FROM users "
                    "WHERE user_id = '%s' "
                ") + 1 WHERE user_id = '%s' ",
            uid.c_str(), uid.c_str());
        sqlres = PQexec(dbconn, sqlquery);

        if (PQresultStatus(sqlres) != PGRES_COMMAND_OK)
        {
            err = PC_DBERROR;
            cerr << PQerrorMessage(dbconn) << endl;
            cerr.flush();
        }
    } else {
        err = PC_DBERROR;

    }

    PQclear(sqlres);

    ret = sys_error(err);
    ret += "\r\n\r\n";

    ret += PQerrorMessage(dbconn);

    return ret;
}
