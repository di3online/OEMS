
/**
 * @file handle_mpsta.cpp
 * @brief
 * @author ZhuZY
 * @version 1.0
 * @date 2012-05-05
 */
#include "handlers.h"
#include "getUIDByCookie.h"
#include "getGIDByUID.h"
#include "common.h"
#include "db.h"

using namespace std;

/**
 * @brief handle_mpsta
 *
 * @param rawtext
 *          The text which contain cookie, pid
 **/

string handle_MPSTA(const string &rawtext)
{
    uid_t userID;
    pid_t pid;
    string cookie = "";
    string status = "true";

    string response = "";

    int err = PC_UNKNOWNERROR;
    DB db;

    size_t start = rawtext.find(' ') + 1;
    size_t end = 0;

    //Get the pid
    end = rawtext.find("\r\n", start);
    pid = rawtext.substr(start, end - start);

    //Get the cookie
    start = rawtext.find(' ', end) + 1;
    end = rawtext.find("\r\n", start);
    cookie = rawtext.substr(start, end - start);

    //get the userID
    PGresult *dbres = PQexec(db.getConn(), "BEGIN");
    if (PQresultStatus(dbres) != PGRES_COMMAND_OK)
    {
        response = sys_error(PC_DBERROR);
        response += "\r\n\r\n";
        PQclear(dbres);
        return response;
    }
    PQclear(dbres);

    userID = getUIDByCookie(cookie, err, db.getConn());
    //check whether the cookie is right
     if (err != PC_SUCCESSFUL)
    {
        response = sys_error(err);
        response += "\r\n\r\n";
        PQexec(db.getConn(), "ROLLBACK");
        return response;
    }

    //check the group id
    gid_t gid = getGIDByUID(userID, err, db.getConn());
    if ((gid != GID_ADMIN) && (gid != GID_TEACHER))
    {
        err = PC_NOPERMISSION;
        response = sys_error(err);
        response += "\r\n\r\n";
        PQexec(db.getConn(), "ROLLBACK");
        return response;
    }

    char sql[300];
    size_t number_pid;
    stringstream ss;

    ss << pid;
    ss >> number_pid;
    
    //Check if there is some questions in the paper.
    snprintf(sql, sizeof(sql), "SELECT * FROM question WHERE paper_id = %lu", number_pid);

    //Exec the SQL query
    PGresult *res;
    res = PQexec(db.getConn(), sql);

    
    if( PQntuples(res) == 0)
    {
        //If there is no question in the paper, 
        //the paper won't be accomplished.

        err = PC_NOTFOUND;
        response = sys_error(err);
        response += "\r\n\r\n";
        PQclear(res);
        PQexec(db.getConn(), "ROLLBACK");
        return response;
    }

    snprintf(sql, sizeof(sql), 
            "UPDATE paper SET status = '%s' WHERE paper_id = %lu", 
            status.c_str(), number_pid);
    res = PQexec(db.getConn(), sql);
    if ((PQresultStatus(res) != PGRES_COMMAND_OK))
    {
        PQclear(res);
        err = PC_SYSTEMERROR;
        response = sys_error(err);
        response += "\r\n\r\n";
        PQexec(db.getConn(), "ROLLBACK");
        return response;
    }

    PQclear(res);
    err = PC_SUCCESSFUL;

    dbres = PQexec(db.getConn(), "COMMIT");
    PQclear(dbres);

    response = sys_error(err);
    response += "\r\n\r\n";

    return response;
}
