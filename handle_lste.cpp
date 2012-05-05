/**
 * @file handle_lstet.cpp
 * @brief 
 * @author LaiSHZH
 * @version 1.0
 * @date 2012-05-02
 */
#include <cstring> 

#include "common.h"
#include "db.h"
#include "xml.h"

#include "getUIDByCookie.h"
#include "getGIDByUID.h"
#include "handlers.h"

using namespace std;

static string parse_result(PGresult *res);

string handle_LSTES(const string &rawtext)
{
    return handle_LSTE(rawtext);
}

string handle_LSTET(const string &rawtext)
{
    return handle_LSTE(rawtext);
}

string 
handle_LSTE(const string & rawtext)
{
    int err;
    string ret;

    const char *strfind = "\r\nCookie: ";
    size_t start = rawtext.find(strfind) + strlen(strfind);

    if (start == string::npos)
    {
        err = PC_INPUTFORMATERROR;
        ret = sys_error(err);
        ret += "\r\n\r\n";
        return ret;
    }

    size_t stop = rawtext.find("\r\n", start);

    if (stop == string::npos)
    {
        err = PC_INPUTFORMATERROR;
        ret = sys_error(err);
        ret += "\r\n\r\n";
        return ret;
    }
    string cookie = rawtext.substr(start, stop - start);

    DB db;
    PGconn *dbconn = db.getConn();
    
    uid_t uid = getUIDByCookie(cookie, err, dbconn);

    gid_t gid = getGIDByUID(uid, err, dbconn);

    PGresult *sqlres;

    if (gid == GID_ADMIN){
        sqlres = PQexec(dbconn, 
                "SELECT owner_id, exam_id, exam_name, course, start_time, end_time, "
                    "exam_id IN ("
                        "SELECT exam_id FROM paper WHERE exam_id NOT IN ("
                            "SELECT exam_id FROM paper WHERE status = false"
                        ")"
                    ")AS accomplishment, management, status FROM exam");

    } else if (gid == GID_TEACHER) {
        char query[MAXBUF];
        snprintf(query, sizeof(query), 
                "SELECT owner_id, exam_id, exam_name, course, start_time, end_time, "
                    "exam_id IN ("
                        "SELECT exam_id FROM paper WHERE exam_id NOT IN ("
                            "SELECT exam_id FROM paper WHERE status = false"
                        ")"
                    ")AS accomplishment, management, status FROM exam WHERE owner_id = '%s'", 
                    uid.c_str());
        sqlres = PQexec(dbconn, query);
        
    } else if (gid == GID_STUDENT) {
        sqlres = PQexec(dbconn, 
                "SELECT owner_id, exam_id, exam_name, course, start_time, end_time, status "
                "FROM exam WHERE management = true");

    } else {
        err = PC_NOPERMISSION;
        ret = sys_error(err);
        ret += "\r\n\r\n";
        return ret;
    }

    if (PQresultStatus(sqlres) == PGRES_TUPLES_OK)
    {
        err = PC_SUCCESSFUL;
        ret = sys_error(err);
        ret += "\r\n\r\n";

        string output = parse_result(sqlres);
        ret += output;
    } else {
        err = PC_DBERROR;
        ret = sys_error(err);
        ret += "\r\n\r\n";
        ret += PQerrorMessage(dbconn);
    }


    PQclear(sqlres);
    
    return ret;
}

static 
string 
parse_result(PGresult *res)
{
    xmlDocPtr doc = NULL;
    xmlNodePtr root_node = NULL;

    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "LSTET");

    xmlDocSetRootElement(doc, root_node);
    for (int i = 0; i < PQntuples(res); ++i)
    {
        xmlNodePtr exam_node 
            = xmlNewNode(NULL, BAD_CAST "exam");
        xmlAddChild(root_node, exam_node);

        for (int j = 0; j < PQnfields(res); ++j)
        {
            const char *col = PQfname(res, j);
            const char *tag = NULL;

            const char *value = PQgetvalue(res, i, j);

            if (strcmp(col, "owner_id") == 0) {
                tag = "owner";
            } else if (strcmp(col, "exam_id") == 0) {
                tag = "eid";
            } else if (strcmp(col, "exam_name") == 0) {
                tag = "name";
            } else if (strcmp(col, "course") == 0) {
                tag = "course";
            } else if (strcmp(col, "start_time") == 0) {
                tag = "stime";
            } else if (strcmp(col, "end_time") == 0) {
                tag = "etime";
            } else if (strcmp(col, "accomplishment") == 0) {
                tag = "accomplishment";
                if (!strcmp(value, "t"))
                {
                    value = "YES";
                } else {
                    value = "NO";
                }
            } else if (strcmp(col, "management") == 0) {
                tag = "management";
                if (!strcmp(value, "f"))
                {
                    value = "offline";
                } else {
                    value = "online";
                }
            } else if (strcmp(col, "status") == 0) {
                tag = "status";
            }

            xmlNewChild(exam_node, NULL, BAD_CAST tag, BAD_CAST value);
        }
    }

    xmlChar *xmlbuffer;
    int buffersize;

    xmlDocDumpFormatMemory(doc, &xmlbuffer, &buffersize, 1);

    string ret = (char *)xmlbuffer;
    
    xmlFree(xmlbuffer);
    xmlFreeDoc(doc);

    return ret;

}
