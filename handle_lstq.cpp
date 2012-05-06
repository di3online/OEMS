#include <vector>
#include <cstdlib>

#include "common.h"
#include "db.h"
#include "xml.h"

using namespace std;

void
getqid(const uid_t &userID, std::vector<qid_t> &questionID, const pid_t &paperID,
        int &err, PGconn *dbconn);

std::string
getdes(const qid_t &questionID,
        int &err, PGconn *dbconn);

void
getchoice(const qid_t &questionID, std::vector<string> &choice,
        int &err, PGconn *dbconn);

cid_t
getkey(const qid_t &questionID,
        int &err, PGconn *dbconn);

std::string
gettime(const qid_t &questionID,
        int &err, PGconn *dbconn);
string
handle_LSTQ(const std::string &rawtext)
{

    uid_t userID;
    pid_t paperID;
    string description;
    string key;
    string time;

    vector<string> questionID;
    size_t start = 0;
    size_t end = rawtext.find(' ');

    start = end+1;
    end = rawtext.find("\r\n",start);
    paperID = rawtext.substr(start,end-start);

    start = end+1;
    end = rawtext.find(' ',start);
    start = end+1;
    end = rawtext.find("\r\n",start);
    string cookie = rawtext.substr(start,end-start);
    int err;
    DB db;
    string response;
    string xml;

    userID = getUIDByCookie(cookie,err,db.getConn());
    if (err != PC_SUCCESSFUL)
    {
        response = sys_error(err);
        response += "\r\n\r\n";
        return response;
    }

    getqid(userID, questionID,paperID,err,db.getConn());
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
    root_node = xmlNewNode(NULL, BAD_CAST "LSTQ");
    xmlNodeSetContent(root_node, BAD_CAST "");

    xmlDocSetRootElement(doc, root_node);

    for(it = questionID.begin();it != questionID.end();it++)
    {
        xmlNodePtr question_node = xmlNewNode(NULL, BAD_CAST "question");
        xmlAddChild(root_node,question_node);
        description = getdes(*it, err, db.getConn());
        if (err != PC_SUCCESSFUL)
        {
            response = sys_error(err);
            response += "\r\n\r\n";
            return response;
        }
        vector<string> choiceID;
        getchoice(*it, choiceID,err,db.getConn());
        if (err != PC_SUCCESSFUL)
        {
            response = sys_error(err);
            response += "\r\n\r\n";
            return response;
        }

        key = getkey(*it,err,db.getConn());
        if (err != PC_SUCCESSFUL)
        {
            response = sys_error(err);
            response += "\r\n\r\n";
            return response;
        }

        time = gettime(*it,err,db.getConn());
        if (err != PC_SUCCESSFUL)
        {
            response = sys_error(err);
            response += "\r\n\r\n";
            return response;
        }

        xmlNewChild(question_node,NULL,BAD_CAST "qid",BAD_CAST it->c_str());
        xmlNewChild(question_node,NULL,BAD_CAST "description",BAD_CAST description.c_str());
        vector<string>::iterator it1;
        for(it1 = choiceID.end()-1;it1>=choiceID.begin();it1--)
        {
            xmlNewChild(question_node,NULL,BAD_CAST "choice",BAD_CAST it1->c_str());
        }
        xmlNewChild(question_node,NULL,BAD_CAST "key",BAD_CAST key.c_str());
        xmlNewChild(question_node,NULL,BAD_CAST "time",BAD_CAST time.c_str());

    }

    xmlChar *xmlbuffer;
    int buffersize;
    xmlDocDumpFormatMemory(doc, &xmlbuffer, &buffersize, 1);

    xml = (char *)xmlbuffer;

    xmlFree(xmlbuffer);
    xmlFreeDoc(doc);
    response = sys_error(PC_SUCCESSFUL);
    response += "\r\n\r\n";
    response += xml;
    return response;
}

void
getqid(const uid_t &userID, vector<string> &questionID, const pid_t &paperID,
        int &err, PGconn *dbconn)
{
    err = PC_UNKNOWNERROR;
    PGconn *conn = dbconn;


    if (PQstatus(conn) != CONNECTION_OK)
    {
        err = PC_DBERROR;
        return ;
    }

    int check_GID = getGIDByUID(userID,err,dbconn);
    if (err != PC_SUCCESSFUL)
    {
        err = PC_DBERROR;
        return ;
    }

    if(check_GID != GID_ADMIN && check_GID != GID_TEACHER)
    {
        err = PC_NOPERMISSION;
        return;
    }

    char cpid[2 * paperID.size() + 1];
    PQescapeString(cpid, paperID.c_str(), paperID.size());

    char sql[300];
    snprintf(sql, sizeof(sql), "SELECT question_id FROM question where paper_id = '%s' ",cpid);
    //Exec the SQL query
    PGresult *res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        err = PC_DBERROR;
        PQclear(res);
        return ;
    }

    int row = PQntuples(res);

    //cout << row<<"+++++++";
    for(;row>0;row--)
    {
        string s = PQgetvalue(res,row-1,0);
        questionID.push_back(s);
    }
    PQclear(res);

    //set the err
    err = PC_SUCCESSFUL;
    return;
}


string
getdes( const std::string &questionID,
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

    char cqid[2 * questionID.size() + 1];
    PQescapeString(cqid, questionID.c_str(), questionID.size());

    char sql[300];
    snprintf(sql, sizeof(sql), "SELECT content FROM question where question_id = '%s' ",cqid);
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

void
getchoice(const std::string &questionID, std::vector<std::string> &choiceID,
        int &err, PGconn *dbconn)
{
    err = PC_UNKNOWNERROR;
    PGconn *conn = dbconn;

    string ret;
    if (PQstatus(conn) != CONNECTION_OK)
    {
        err = PC_DBERROR;
        return ;
    }

    char cqid[2 * questionID.size() + 1];
    PQescapeString(cqid, questionID.c_str(), questionID.size());

    char sql[300];
    snprintf(sql, sizeof(sql), "SELECT answer_content FROM choice where question_id = '%s' ",cqid);
    //Exec the SQL query
    PGresult *res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        err = PC_DBERROR;
        PQclear(res);
        return ;
    }

    int row = PQntuples(res);
    //cout << row<<"+++++++";
    for(;row>0;row--)
    {
        string s = PQgetvalue(res,row-1,0);
        choiceID.push_back(s);
    }

    PQclear(res);
    err = PC_SUCCESSFUL;

    return ;
}

string
getkey(const std::string &questionID,
        int &err, PGconn *dbconn)
{
    err = PC_UNKNOWNERROR;
    PGconn *conn = dbconn;

    string ret;
    vector<string> choiceContent;
    if (PQstatus(conn) != CONNECTION_OK)
    {
        ret = "";
        err = PC_DBERROR;
        return ret;
    }

    char cqid[2 * questionID.size() + 1];
    PQescapeString(cqid, questionID.c_str(), questionID.size());

    char sql[300];
    snprintf(sql, sizeof(sql), "SELECT key FROM question where question_id = '%s' ",cqid);
    //Exec the SQL query
    PGresult *res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        ret ="";
        err = PC_DBERROR;
        PQclear(res);
        return ret;
    }

    string k = PQgetvalue(res,0,0);
    PQclear(res);

    snprintf(sql, sizeof(sql), "SELECT answer_content FROM choice where question_id = '%s' ",cqid);
    res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        ret ="";
        err = PC_DBERROR;
        PQclear(res);
        return ret;
    }

    int row = PQntuples(res);
    //cout << row<<"+++++++";
    for(;row>0;row--)
    {
        string s = PQgetvalue(res,row-1,0);
        choiceContent.push_back(s);
    }

    snprintf(sql, sizeof(sql), "SELECT answer_content FROM choice where choice_id = '%s' ",k.c_str());
    res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        ret ="";
        err = PC_DBERROR;
        PQclear(res);
        return ret;
    }
    string c = PQgetvalue(res,0,0);
    PQclear(res);

    vector<string>::iterator it;
    int num = 1;
    for(it = choiceContent.end()-1;it>=choiceContent.begin();it--,num++)
    {
        if(*it == c)
            break;
    }

    char t[256];
    sprintf(t, "%d", num);
    ret = t;
    err = PC_SUCCESSFUL;

    return ret;
}

string
gettime(const std::string &questionID,
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

    char cqid[2 * questionID.size() + 1];
    PQescapeString(cqid, questionID.c_str(), questionID.size());

    char sql[300];
    snprintf(sql, sizeof(sql), "SELECT timelimit FROM question where question_id = '%s' ",cqid);
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
