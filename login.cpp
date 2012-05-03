/**
 * @file login.cpp
 * @brief 
 * @author LaiSHZH
 * @version 1.1
 * @date 2012-04-08
 */
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include "common.h"
#include "db.h"
#include "xml.h"
#include "login.h"
#include "getGIDByUID.h"


//Five hours
const static unsigned int CONST_LOGIN_INTERVAL = 60;// * 60 * 12;

using namespace std;

bool check_cookie(char *);
char *generate_cookie(char *, size_t, char *);
/**
 * @brief handle_login 
 *
 * @param rawtext
 *          The text which contain userID and password
 * @param userID
 *          The user ID will return
 * @param password
 *          The password will return
 */
string 
handle_login(const string &rawtext)
{
    int err;
    string response;
    
    uid_t userID;
    string password;

    try {
        size_t start = 0;
        size_t end = rawtext.find(' ');
        
        if (string::npos == end)
        {
            throw 1;
        }
        
        //ingore the login identifier

        //Get the userID
        start = end + 1;
        end = rawtext.find(' ', start);
        if (string::npos == end)
        {
            throw 2;
        }
        userID = rawtext.substr(start, end - start);

        //Get the password
        start = end + 1;
        end = rawtext.find_first_of(" \r\n", start);
        if (string::npos == end)
        {
            throw 3;
        }
        password = rawtext.substr(start, end - start);

    } catch (int ) {

    }
  
    //Init the db connection 
    DB db;
    PGresult *dbres = PQexec(db.getConn(), "BEGIN");

    if (PQresultStatus(dbres) != PGRES_COMMAND_OK)
    {
        response = sys_error(PC_DBERROR);
        response += "\r\n\r\n";
        PQclear(dbres);
        return response;
    }
    PQclear(dbres);

    string cookie = login(userID, password, err, db.getConn());
    
    //Check the return code of the function
    if (err != PC_SUCCESSFUL)
    {
        response = sys_error(err);
        response += "\r\n\r\n";
        PQexec(db.getConn(), "ROLLBACK");
        return response;
    }

    gid_t gid = getGIDByUID(userID, err, db.getConn());

    const char *group = echo_gid(gid);

    if (err != PC_SUCCESSFUL)
    {
        response = sys_error(err);
        response += "\r\n\r\n";
        PQexec(db.getConn(), "ROLLBACK");
        return response;
    }

    dbres = PQexec(db.getConn(), "COMMIT");
    if (PQresultStatus(dbres) != PGRES_COMMAND_OK)
    {
        response = sys_error(PC_DBERROR);
        response += "\r\n\r\n";
        PQexec(db.getConn(), "ROLLBACK");
        return response;
    }

    response = sys_error(PC_SUCCESSFUL);
    response += "\r\n\r\n";

    //Generate the XML body
    xmlDocPtr doc = NULL;
    xmlNodePtr root_node = NULL;

    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "login");

    xmlDocSetRootElement(doc, root_node);
    xmlNewChild(root_node, NULL, BAD_CAST "gid", 
                        BAD_CAST group);
    
    xmlNewChild(root_node, NULL, BAD_CAST "cookie", 
                        BAD_CAST cookie.c_str());


    xmlChar *xmlbuffer;
    int buffersize;

    xmlDocDumpFormatMemory(doc, &xmlbuffer, &buffersize, 1);

    response += (char *)xmlbuffer;


    xmlFree(xmlbuffer);
    xmlFreeDoc(doc);

    return response;

}


/**
 * @brief login 
 *          Check the userID and password, 
 *          and return Cookie of the user if succeed
 * @param userID 
 *          User's ID which will be checked
 * @param password
 *          User's password will be checked
 * @param err
 *          return the status of the function progress
 * @param dbconn
 *          the connection to database which is created in 
 *          the caller.
 *
 * @return 
 *          Cookie and group of the user if user id matches its password.
 */

string 
login(const uid_t &userID, const string &password, int &err, PGconn *dbconn)
{

    err = PC_UNKNOWNERROR;
    PGconn *conn = dbconn;

    string ret;
    
    if (PQstatus(conn) != CONNECTION_OK)
    {
        ret = "";
        err = PC_DBERROR;
        return ret;
    }

    if (userID.size() > MAXLEN_USERID)
    {
        err = PC_INPUTFORMATERROR;
        ret = "";
        return ret;
    }

    if (password.size() > MAXLEN_PASSWORD)
    {
        err = PC_INPUTFORMATERROR;
        ret = "";
        return ret;
    }

    //escapte string of userID and password
    char cuid[2 * userID.size() + 1];
    char cpasswd[2 * password.size() + 1];

    PQescapeString(cuid, userID.c_str(), userID.size());
    PQescapeString(cpasswd, password.c_str(), password.size());

    char sql[300];
    snprintf(sql, sizeof(sql), 
            "SELECT cookie, group_id FROM users WHERE user_id='%s' and password='%s'", 
            cuid, cpasswd);

    //Exec the SQL query
    PGresult *res = NULL;
    res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {//If exection failed
        err = PC_DBERROR;
        ret = "";
        PQclear(res);
        return ret;
    }

    if (PQntuples(res) == 0)
    {//If userID doesn't match password
        err = PC_MISTATCH;
        ret = "";
        PQclear(res);
        return ret;
    }


    char *cookie = (char *) malloc(strlen(PQgetvalue(res, 0, 0)) + 1);

    memmove(cookie, PQgetvalue(res, 0, 0), strlen(PQgetvalue(res, 0, 0)) + 1);

    PQclear(res);

    if (check_cookie(cookie))
    {
        //size of cookie
        cookie = (char *) realloc(cookie, MAXLEN_COOKIE);
        bool flag_update = true;
        while ( flag_update )
        {
            generate_cookie(cookie, 100, cuid);

            snprintf(sql , sizeof(sql), "UPDATE users SET cookie = '%s' WHERE user_id = '%s' RETURNING cookie", cookie, cuid);

            res = PQexec(conn, sql);
            if (PQresultStatus(res) == PGRES_TUPLES_OK)
            {
                flag_update = false;
                memcpy(cookie, PQgetvalue(res, 0, 0), strlen(PQgetvalue(res, 0, 0)) + 1);
            }
            
        }
        PQclear(res);
    }


    err = PC_SUCCESSFUL;
    ret = cookie;

    free(cookie);
    return ret;
}

bool 
check_cookie(char *cookie)
{
    time_t cur_time;
    time(&cur_time);

    time_t cookie_time;
    char *pos = strchr(cookie, '-');

    if (pos)
    {
        char *stop = NULL;
        cookie_time = strtoul(cookie, &stop, 0);
        if (stop < pos)
        {
            return true;
        }

        if (fabs(difftime(cur_time, cookie_time)) > CONST_LOGIN_INTERVAL)
        {
            return true;
        }
    } else {
        return true;
    }

    return false;
}

char *
generate_cookie(char *cookie, size_t size, char *cuid)
{
    time_t cur_time;
    time(&cur_time);

    srand(cur_time);
    unsigned long random = rand() % 100000L;

    snprintf(cookie, size, "%lu-%lu-%s", cur_time, random, cuid);

    return cookie;
}
