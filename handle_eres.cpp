#include "handlers.h"

#include <sstream>

/**
 * @file handle_eres.cpp
 * @brief
 * @author ZhuZY
 * @version 1.1
 * @date 2012-05-06
 */

using namespace std;

/**
 * @brief handle_eres
 *
 * @param rawtext
 *          The text which contain cookie, eid and the start time
 * @param userID
 *          The uid
 * @param eid
 *          The eid
 * @param pid
 *          The pid
 */
string handle_ERES(const string &rawtext)
{
    uid_t userID;
    eid_t eid;
    string cookie = "";
    string response = "";

    int err = PC_UNKNOWNERROR;
    DB db;

    size_t start = rawtext.find(' ') + 1;
    size_t end = 0;

    //Get the eid
    end = rawtext.find("\r\n", start);
    eid = rawtext.substr(start, end - start);

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
    size_t number_eid;
    stringstream ss;

    ss << eid;
    ss >> number_eid;
    snprintf(sql, sizeof(sql), "SELECT paper_id FROM paper WHERE exam_id = %lu", number_eid);
    ss.clear();
    ss.str("");


    //Exec the SQL query
    PGresult *res;
    res = PQexec(db.getConn(), sql);
    if ((PQresultStatus(res) != PGRES_TUPLES_OK))
    {
        err = PC_DBERROR;
        response = sys_error(err);
        response += "\r\n\r\n";
        PQclear(res);
        PQexec(db.getConn(), "ROLLBACK");
        return response;
    }
    //check if the eid is exist
    if( PQntuples(res) != 3)
    {
        err = PC_NOTFOUND;
        response = sys_error(err);
        response += "\r\n\r\n";
        PQclear(res);
        PQexec(db.getConn(), "ROLLBACK");
        return response;
    }

    xmlDocPtr doc = NULL;
    xmlNodePtr root_node1 = NULL;
    xmlNodePtr root_node2 = NULL;

    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node1 = xmlNewNode(NULL, BAD_CAST "ERES");

    xmlDocSetRootElement(doc, root_node1);

    pid_t paper[3];
    int number_pid[3];
    int i = 0;
    int j = 0;
    for(i = 0; i < 3; i++)
    {
        paper[i] = PQgetvalue(res, i, 0);
        ss << paper[i];
        ss >> number_pid[i];
        ss.clear();
        ss.str("");
    }

    int number_student = 0;
    uid_t student_uid;
    string grade ="";
    string student_name = "";
    int number_paper;
    stringstream ssd;
    string paper_string = "";
    PGresult *resi;
    PGresult *resj;

    for (i = 0; i < 3; i++)
    {
        snprintf(sql, sizeof(sql), "SELECT user_id, final_score FROM student_exam WHERE paper_id = %d", number_pid[i]);
        resi = PQexec(db.getConn(), sql);

        if ((PQresultStatus(resi) != PGRES_TUPLES_OK))
        {
            err = PC_DBERROR;
            response = sys_error(err);
            response += "\r\n\r\n";
            PQclear(resi);
            PQexec(db.getConn(), "ROLLBACK");
            return response;
        }
        number_student = PQntuples(resi);

        for(j = 0; j < number_student; j++)
        {

             root_node2 = xmlNewNode(NULL, BAD_CAST "student");
             xmlAddChild(root_node1, root_node2);
             student_uid = PQgetvalue(resi, j, 0);
             grade = PQgetvalue(resi, j , 1);
             snprintf(sql, sizeof(sql), "SELECT name FROM users WHERE user_id = '%s'", student_uid.c_str());
             resj = PQexec(db.getConn(), sql);
             if ((PQresultStatus(resj) != PGRES_TUPLES_OK))
            {
                err = PC_DBERROR;
                response = sys_error(err);
                response += "\r\n\r\n";
                PQclear(resj);
                PQexec(db.getConn(), "ROLLBACK");
                return response;
            }

            student_name = PQgetvalue(resj, 0, 0);
            number_paper = i + 1;
            ssd << number_paper;
            paper_string = ssd.str();
            ssd.clear();
            ssd.str("");

            //char paper_char[20];
            //snprintf(paper_char, sizeof(paper_char), "%d", number_paper);

            xmlNewChild(root_node2, NULL, BAD_CAST "name",
                        BAD_CAST student_name.c_str());
            xmlNewChild(root_node2, NULL, BAD_CAST "uid",
                        BAD_CAST student_uid.c_str());
            xmlNewChild(root_node2, NULL, BAD_CAST "grade",
                        BAD_CAST grade.c_str());
            xmlNewChild(root_node2, NULL, BAD_CAST "paper",
                        //(const xmlChar *) paper_string.c_str());
                        BAD_CAST paper_string.c_str());

            PQclear(resj);
        }
        PQclear(resi);
    }

    xmlChar *xmlbuffer;
    int buffersize;

    xmlDocDumpFormatMemory(doc, &xmlbuffer, &buffersize, 1);

    dbres = PQexec(db.getConn(), "COMMIT");

    PQclear(res);
    err = PC_SUCCESSFUL;
    response = sys_error(err);
    response += "\r\n\r\n";
    response += (char *)xmlbuffer;

    xmlFree(xmlbuffer);
    xmlFreeDoc(doc);

    return response;
}
