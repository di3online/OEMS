/**
 * @file getCIDByUID.cpp
 * @brief
 * @author MoShuqi
 * @version 1.0
 * @date 2012-04-12
 */
#include "getGIDByUID.h"
#include <sstream>

string handle_getGIDByUID(std::string &rawtext)
{
    size_t start = 0;
    size_t end = rawtext.find(' ');

    uid_t userID;
    string response;

    //ingore the GID identifier

    //Get the userID
    start = end + 1;
    end = rawtext.find_first_of("\r\n", start);

    userID = rawtext.substr(start, end - start);
    cout<<"The userID id: "<<userID<<endl;

    int err;
    DB db;

    gid_t gid = getGIDByUID(userID, err, db.getConn());

    if (err != PC_SUCCESSFUL)
    {
        response = sys_error(err);
        //cout<<"this"<<response<<"the gid is:"<<gid<<endl;
        response += "\r\n\r\n";
        return response;
    }

    response = sys_error(PC_SUCCESSFUL);
    response += "\r\n";

    //int gid = getGIDByUID(userID);
    std::ostringstream os;
    os<<gid;

    response += "GID: ";
    response += os.str();
    response += "\r\n\r\n";

    return response;

}

/**
 * @brief getGIDByUID
 *
 * @param userID
 *          Get a group id by the userID
 * @return
 *          The group id of the userID
 */


gid_t getGIDByUID(const uid_t &userID,int &err, PGconn *dbconn)
{


    err = PC_UNKNOWNERROR;
    PGconn *conn = dbconn;

    gid_t groupID = GID_UNKNOWN;

    if (PQstatus(conn) != CONNECTION_OK)
    {
        err = PC_DBERROR;
        return groupID;
    }

    if (userID.size() > MAXLEN_USERID)
    {
        err = PC_INPUTFORMATERROR;
        return groupID;
    }

    char cuid[2 * userID.size() + 1];
    PQescapeString(cuid, userID.c_str(), userID.size());

    string t = cuid;

    string sql = "SELECT group_id FROM users WHERE user_id ='"+t+"'";

    //Exec the SQL query
    PGresult *res = NULL;
    res = PQexec(conn, sql.c_str());

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {//If exection failed
        err = PC_DBERROR;
        PQclear(res);
        return groupID;
    }

    if (PQntuples(res) == 0)
    {//If userID doesn't match password
        err = PC_MISTATCH;
        PQclear(res);
        return groupID;
    }

    string s = PQgetvalue(res, 0, 0);

    //make the string to int
    std::istringstream in(s);
    in>>groupID;
    err = PC_SUCCESSFUL;

    //Don't forget to clear the result of query at end.
    //Updated By: Lai
    PQclear(res);

    switch (groupID)
    {
        case 0:
            groupID = GID_ADMIN;
            break;
        case 1:
            groupID = GID_TEACHER;
            break;
        case 2:
            groupID = GID_STUDENT;
            break;
        default:
            groupID = GID_UNKNOWN;
            break;
    }

    return groupID;

}
