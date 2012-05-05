/**
 * @file handle_einf.cpp
 * @brief 
 * @author LaiSHZH
 * @version 1.0
 * @date 2012-05-03
 */

#include <cstring>

#include "common.h"
#include "db.h"
#include "xml.h"

#include "getUIDByCookie.h"
#include "getGIDByUID.h"
#include "handlers.h"

string 
handle_EINF(const string &rawtext)
{
    int err;
    string ret;

    const char *handlefind = "EINF ";
    const char *strfind = "\r\nCookie: ";

    size_t start = rawtext.find(handlefind) + strlen(handlefind);

    if (start == string::npos)
    {
        err = PC_INPUTFORMATERROR;
        ret = sys_error(err);
        ret += "\r\n\r\n";
        return ret;
    }

    size_t stop = rawtext.find(strfind, start);

    if (stop == string::npos)
    {
        err = PC_INPUTFORMATERROR;
        ret = sys_error(err);
        ret += "\r\n\r\n";
        return ret;
    }

    eid_t eid = rawtext.substr(start, stop - start);

    start = stop + strlen(strfind);

    stop = rawtext.find("\r\n", start);

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

    if (gid == GID_ADMIN || gid == GID_TEACHER)
    {
        char ceid[MAXBUF];
        PQescapeString(ceid, eid.c_str(), eid.size() + 1);

        char sql_buf[MAXBUF];
        PGresult *res_exam;
        snprintf(sql_buf, sizeof(sql_buf), 
                    "SELECT owner_id, exam_id, exam_name, course, start_time, end_time, "
                    "exam_id IN ("
                        "SELECT exam_id FROM paper WHERE exam_id NOT IN ("
                            "SELECT exam_id FROM paper WHERE status = false"
                        ")"
                    ")AS accomplishment, management, status FROM exam "
                    " WHERE exam_id = '%s'", ceid);
        res_exam = PQexec(dbconn, sql_buf);

        if (PQresultStatus(res_exam) != PGRES_TUPLES_OK)
        {
            err = PC_DBERROR;
            ret = sys_error(err);
            ret += "\r\n\r\n";
            PQclear(res_exam);
            return ret;
        }

        if (PQntuples(res_exam) == 0)
        {
            err = PC_NOTFOUND;
            ret = sys_error(err);
            ret += "\r\n\r\n";
            PQclear(res_exam);
            return ret;
        }
        PGresult *res_paper;

        snprintf(sql_buf, sizeof(sql_buf), "SELECT paper_id, status FROM paper WHERE exam_id = '%s' ORDER BY paper_id", ceid); 
        
        res_paper = PQexec(dbconn, sql_buf);

        if (PQresultStatus(res_paper) != PGRES_TUPLES_OK)
        {
            err = PC_DBERROR;
            ret = sys_error(err);
            ret += "\r\n\r\n";

            //ret += PQerrorMessage(dbconn);
            PQclear(res_exam);
            PQclear(res_paper);
            return ret;
        }

        xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
        xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "EINF");
        xmlDocSetRootElement(doc, root_node);

        for (int j = 0; j < PQnfields(res_exam); ++j)
        {
            const char *col = PQfname(res_exam, j);
            const char *tag = NULL;

            const char *value = PQgetvalue(res_exam, 0, j);

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

            xmlNewChild(root_node, NULL, BAD_CAST tag, BAD_CAST value);
        }

        for (int i = 0; i < PQntuples(res_paper); ++i)
        {
            xmlNodePtr paper_node = xmlNewNode(NULL, BAD_CAST "paper");
            xmlAddChild(root_node, paper_node);
            xmlNewChild(paper_node, NULL, BAD_CAST "pid", BAD_CAST PQgetvalue(res_paper, i, 0));

            if (!strcmp(PQgetvalue(res_paper, i, 1), "t"))
            {
                xmlNewChild(paper_node, NULL, BAD_CAST "status", BAD_CAST "Y");
            } else {
                xmlNewChild(paper_node, NULL, BAD_CAST "status", BAD_CAST "N");
            }
        }




        PQclear(res_exam);
        PQclear(res_paper);
        err = PC_SUCCESSFUL;
        ret = sys_error(err);
        ret += "\r\n\r\n";

        xmlChar *xmlbuffer;
        int buffersize;

        xmlDocDumpFormatMemory(doc, &xmlbuffer, &buffersize, 1);

        ret += (char *) xmlbuffer;

        xmlFree(xmlbuffer);
        xmlFreeDoc(doc);
        
        return ret;
    } else {
        err = PC_NOPERMISSION;
        ret = sys_error(err);
        ret += "\r\n\r\n";
        return ret;
    }
}
