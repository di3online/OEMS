
#include "handlers.h"
#include "common.h"
#include "xml.h"
#include "db.h"

#include <iostream>

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

    char sql_buf[MAXBUF];

    snprintf(sql_buf, sizeof(sql_buf),
            "SELECT e.exam_name as name, "
                "e.course as course, "
                "p.paper_id as pid, "
                "s.final_score as score "
            "FROM exam e "
            "INNER JOIN paper p ON e.exam_id = p.exam_id "
            "INNER JOIN student_exam s ON s.paper_id = p.paper_id "
            "WHERE s.user_id = '%s'",
            userID.c_str());

    PGresult *res = PQexec(conn, sql_buf);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        err = PC_DBERROR;
        response = sys_error(err);
        response += "\r\n\r\n";
        response += PQerrorMessage(conn);
        PQclear(res);
        return response;
    }

    response = sys_error(PC_SUCCESSFUL);
    response += "\r\n\r\n";

    xmlDocPtr doc = NULL;
    xmlNodePtr root_node = NULL;

    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "LSTERS");
    xmlNodeSetContent(root_node, BAD_CAST "");

    xmlDocSetRootElement(doc, root_node);

    for (int i = 0; i < PQntuples(res); ++i)
    {
        xmlNodePtr exam_node = xmlNewNode( NULL, BAD_CAST "exam");

        xmlNewChild(exam_node, NULL, BAD_CAST "pid",    BAD_CAST PQgetvalue(res, i, 2));
        xmlNewChild(exam_node, NULL, BAD_CAST "name",   BAD_CAST PQgetvalue(res, i, 0));
        xmlNewChild(exam_node, NULL, BAD_CAST "course", BAD_CAST PQgetvalue(res, i, 1));

        if ( !PQgetisnull(res, i, 3))
        {
            xmlNewChild(exam_node, NULL, BAD_CAST "score",  BAD_CAST PQgetvalue(res, i, 3));
        } else {
            // TODO:regrade the paper of this student
            xmlNewChild(exam_node, NULL, BAD_CAST "score", BAD_CAST "");
        }

        xmlAddChild(root_node, exam_node);
    }

    xmlChar *xmlbuffer;
    int buffersize;

    xmlDocDumpFormatMemory(doc, &xmlbuffer, &buffersize, 1);

    response += (char *)xmlbuffer;

    xmlFree(xmlbuffer);
    xmlFreeDoc(doc);
    
    PQclear(res);

    return response;
 }
