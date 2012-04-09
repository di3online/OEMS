/**
 * @file login.cpp
 * @brief 
 * @author LaiSHZH
 * @version 1.0
 * @date 2012-04-08
 */
#include "login.h"
#include "db.h"
#include "xml.h"

using namespace std;

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
void handle_login(const string &rawtext, uid_t &userID, string &password)
{
    size_t start = 0;
    size_t end = rawtext.find(' ');
    
    //ingore the login identifier

    //Get the userID
    start = end + 1;
    end = rawtext.find(' ', start);

    userID = rawtext.substr(start, end - start);

    //Get the password
    start = end + 1;
    end = rawtext.find('\r', start);
    
    password = rawtext.substr(start, end - start);

    return ;
}


/**
 * @brief login 
 *          Check the userID and password, 
 *          and return Cookie of the user if succeed
 * @param userID 
 *          User's ID which will be checked
 * @param password
 *          User's password will be checked
 *
 * @return 
 *          The string which contain Cookie 
 *          if userID and password is correct
 */

string 
login(const uid_t &userID, const string &password)
{
    DB db;
    PGconn *conn = db.getConn();

    string ret;
    
    if (!db.isConnected())
    {
        ret = "500 System error\r\n\r\n";
        return ret;
    }

    if (userID.size() > MAXLEN_USERID)
    {
        ret = "500 userID is too long\r\n\r\n";
        return ret;
    }

    if (password.size() > MAXLEN_PASSWORD)
    {
        ret = "500 password is too long\r\n\r\n";
        return ret;
    }

    //escapte string of userID and password
    char cuid[2 * userID.size() + 1];
    char cpasswd[2 * password.size() + 1];

    PQescapeString(cuid, userID.c_str(), userID.size());
    PQescapeString(cpasswd, password.c_str(), password.size());

    char sql[300];
    snprintf(sql, sizeof(sql), "SELECT cookie FROM users WHERE user_id='%s' and password='%s'", cuid, cpasswd);

    //Exec the SQL query
    PGresult *res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {//If exection failed
        ret = "500 System error\r\n\r\n";
        PQclear(res);
        return ret;
    }

    if (PQntuples(res) == 0)
    {//If userID doesn't match password
        ret = "403 MISMATCH\r\n\r\n";
        PQclear(res);
        return ret;
    }


    ret = "200 OK\r\n";
    ret += "\r\n";

    //Generate the XML body
    xmlDocPtr doc = NULL;
    xmlNodePtr root_node = NULL, node = NULL;

    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "LOGIN");

    xmlDocSetRootElement(doc, root_node);
    node = xmlNewChild(root_node, NULL, BAD_CAST "cookie", BAD_CAST PQgetvalue(res, 0, 0));
    
    xmlChar *xmlbuffer;
    int buffersize;

    xmlDocDumpFormatMemory(doc, &xmlbuffer, &buffersize, 1);

    ret += (char *)xmlbuffer;

    //Free the result of SQL
    PQclear(res);

    xmlFree(xmlbuffer);
    xmlFreeDoc(doc);


    return ret;
}
