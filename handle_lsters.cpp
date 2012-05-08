
#include "handlers.h"
#include "common.h"
#include "xml.h"
#include "db.h"

#include <iostream>


static string
getPID( const std::string &userID,
        int &err, PGconn *dbconn);


static string
getName( const std::string &userID,
        int &err, PGconn *dbconn);

static string
getScore( const std::string &userID,
        int &err, PGconn *dbconn);

/**
 * @brief handle_lsters
 *
 * @param rawtext
 *          The text contains cookie which can resolve userID,questionID and choiceID
 * @param userID
 *          The user ID,question ID and choice ID will return
 */
 string handle_LSTERS(const std::string &rawtext)
 {
    uid_t userID;
    qid_t questionID;
    cid_t choiceID;
    pid_t paperID;
    string temp_get;
    // define the return value
    string response;

    string score = "";
    string name;
    string check_LSTERS = "";

    size_t start = 0;
    size_t end = rawtext.find("\r\n");

    check_LSTERS = rawtext.substr(start, end - start);
    //check the method is LSTERS
    if (check_LSTERS != "LSTERS")
    {
        return "you called a wrong method which is not LSTERS!\n";
    }

    //Init the err
    int err = PC_UNKNOWNERROR;
    //Init the db connection
    DB db;
    PGconn *conn = db.getConn();

    // ignore the "cookie: "
    start = end + 2;
    end = rawtext.find(' ', start);

    //Get the userID
    start = end + 1;
    end = rawtext.find("\r\n\r\n", start);
    temp_get= rawtext.substr(start, end - start);
    userID = getUIDByCookie(temp_get,err,conn);
    //Check the return code of the function
    if (err != PC_SUCCESSFUL)
    {
        response = sys_error(err);
        response += "\r\n\r\n";
        return response;
    }

    //check the db status
    if (PQstatus(conn) != CONNECTION_OK)
    {
        response = PC_DBERROR;
        response += "\r\n\r\n";
        return response;
    }

    //get paper ID
    paperID = getPID(userID, err, conn);
    if (err != PC_SUCCESSFUL)
    {
        response = sys_error(err);
        response += "\r\n\r\n";
        return response;
    }

    //get name
    name = getName(userID, err, db.getConn());
    if (err != PC_SUCCESSFUL)
    {
        response = sys_error(err);
        response += "\r\n\r\n";
        return response;
    }

    //get score
    score = getScore(userID, err, conn);
    if (err != PC_SUCCESSFUL)
    {
        response = sys_error(err);
        response += "\r\n\r\n";
        return response;
    }


    vector<string>::iterator it;
    /*
    for (it=questionID.begin(); it<questionID.end(); it++)
    {
    cout << " " << *it;
    cout << endl;
    }*/

    xmlDocPtr doc = NULL;
    xmlNodePtr root_node = NULL;

    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "LSTERS");

    xmlDocSetRootElement(doc, root_node);

    xmlNodePtr exam_node = xmlNewNode( NULL, BAD_CAST "exam");

    xmlNewChild(exam_node, NULL, BAD_CAST "pid",BAD_CAST paperID.c_str());

    xmlNewChild(exam_node, NULL, BAD_CAST "name",BAD_CAST name.c_str());

    xmlNewChild(exam_node, NULL, BAD_CAST "score",BAD_CAST score.c_str());

    xmlAddChild(root_node, exam_node);


    //xmlNodePtr paper_node = xmlNewNode(NULL, BAD_CAST "exam");
    //xmlNewChild(paper_node,NULL,BAD_CAST "pid",BAD_CAST it->c_str());

    /*xmlNodePtr name_node = xmlNewNode(NULL, BAD_CAST "exam");
    xmlNewChild(name_node,NULL,BAD_CAST "name",BAD_CAST name.c_str());

    xmlNodePtr score_node = xmlNewNode(NULL, BAD_CAST "exam");
    xmlNewChild(score_node,NULL,BAD_CAST "score",BAD_CAST score.c_str());*/

   // xmlAddChild(root_node,paper_node);


    xmlChar *xmlbuffer;
    int buffersize;

    xmlDocDumpFormatMemory(doc, &xmlbuffer, &buffersize, 1);

    temp_get = (char *)xmlbuffer;

    xmlFree(xmlbuffer);
    xmlFreeDoc(doc);
    response = sys_error(PC_SUCCESSFUL);
    response += "\r\n\r\n";
    response += temp_get;
    return response;


 }

string
getPID( const std::string &userID,
        int &err, PGconn *dbconn)
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

    char cuid[2 * userID.size() + 1];
    PQescapeString(cuid, userID.c_str(), userID.size());

    char sql[300];
    snprintf(sql, sizeof(sql), "SELECT paper_id FROM student_exam where user_id = '%s' ",cuid);
    //Exec the SQL query
    PGresult *res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        ret ="";
        err = PC_DBERROR;
        PQclear(res);
        return ret;
    }

    ret = PQgetvalue(res,0,0);
    PQclear(res);
    err = PC_SUCCESSFUL;

    return ret;
}

string
getName( const std::string &userID,
        int &err, PGconn *dbconn)
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

    char cuid[2 * userID.size() + 1];
    PQescapeString(cuid, userID.c_str(), userID.size());

    char sql[300];
    snprintf(sql, sizeof(sql), "SELECT name FROM users where user_id = '%s' ",cuid);
    //Exec the SQL query
    PGresult *res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        ret ="";
        err = PC_DBERROR;
        PQclear(res);
        return ret;
    }

    ret = PQgetvalue(res,0,0);
    PQclear(res);
    err = PC_SUCCESSFUL;

    return ret;
}

string
getScore( const std::string &userID,
        int &err, PGconn *dbconn)
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

    char cuid[2 * userID.size() + 1];
    PQescapeString(cuid, userID.c_str(), userID.size());

    char sql[300];
    snprintf(sql, sizeof(sql), "SELECT final_score FROM student_exam where user_id = '%s' ",cuid);
    //Exec the SQL query
    PGresult *res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        ret ="";
        err = PC_DBERROR;
        PQclear(res);
        return ret;
    }

    ret = PQgetvalue(res,0,0);
    PQclear(res);
    err = PC_SUCCESSFUL;

    return ret;
}
