
#include "common.h"
#include "handlers.h"


/**
 * @brief listExamReult
 *          Check the cookie
 *          and list one exam reult
 *          of one student
 * @param paper_id
 *         Paper's ID which will be listed
 *
 * @return
 *          list OK
 *          if paper_id is right
 */

string
handle_SEDT(const std::string &rawtext)
{
    string paperID;
    string cookie;

    size_t start = 0;
    size_t end = rawtext.find(' ');

    //ingore the SEDT identifier

    //Get the paperID
    start = end + 1;
    end = rawtext.find("\r\n", start);
    paperID = rawtext.substr(start, end - start);

    ////ingore the COOKIE identifier
    start = end + 1;
    end = rawtext.find(' ', start);

    //Get the cookie
    start = end + 1;
    end = rawtext.find("\r\n\r\n", start);

    cookie = rawtext.substr(start, end - start);

    int err;
    string response;

    DB db;


    if (cookie.size() > MAXLEN_COOKIE)
    {
        response = sys_error(PC_INPUTFORMATERROR);
        response += "\r\n\r\n";
        return response;
    }

    if (paperID.size() > MAXLEN_PID)
    {
        response = sys_error(PC_INPUTFORMATERROR);
        response += "\r\n\r\n";
        return response;
    }


    string uid = getUIDByCookie(cookie,err,db.getConn());

    //Check the return code of the function
    if (err != PC_SUCCESSFUL)
    {
        response = sys_error(err);
        response += "\r\n\r\n";
        return response;
    }

    //listExamReult(uid, paperID, err, db.getConn());

    err = PC_UNKNOWNERROR;
    gid_t gid = getGIDByUID(uid, err, db.getConn());

    if (err != PC_SUCCESSFUL)
    {
        response = sys_error(err);
        response += "\r\n\r\n";
        return response;
    }

    if (gid != GID_STUDENT)
    {
        //If user is not expected
        response = sys_error(PC_NOPERMISSION);
        response += "\r\n\r\n";
        return response;
    }


    char cpid[2 * paperID.size() + 1];
    char cuid[2 * uid.size() + 1];

    PQescapeString(cpid, paperID.c_str(), paperID.size());
    PQescapeString(cuid, uid.c_str(), uid.size());

    
    char sql_buf[MAXBUF];

    snprintf(sql_buf, sizeof(sql_buf), 
            "SELECT q.content AS content, key.answer_content, sheetB.stuanswer AS keyanswer FROM question q "
            "INNER JOIN choice key ON q.paper_id = %s AND q.key = key.choice_id "
            "LEFT JOIN ( "
                "SELECT answer_sheet.question_id AS qid, choice.answer_content AS stuanswer FROM answer_sheet "
                "INNER JOIN choice ON answer_sheet.answer = choice.choice_id AND answer_sheet.user_id = '%s' "
            ") AS sheetB ON q.question_id = sheetB.qid ORDER BY q.question_id ",
            cpid, uid.c_str());
    PGresult *res = PQexec(db.getConn(), sql_buf);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        err = PC_DBERROR;
        response = sys_error(err);
        response += "\r\n\r\n";

        PQclear(res);
        return response;
    }


    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");

    xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "SEDT");
    xmlNodeSetContent(root_node, BAD_CAST "");
    xmlDocSetRootElement(doc, root_node);

    for (int i = 0; i < PQntuples(res); ++i)
    {
        xmlNodePtr q_node = xmlNewNode(NULL, BAD_CAST "question");
        xmlAddChild(root_node, q_node);

        xmlNewChild(q_node, NULL, BAD_CAST "content", BAD_CAST PQgetvalue(res, i, 0));
        xmlNewChild(q_node, NULL, BAD_CAST "cAnswer", BAD_CAST PQgetvalue(res, i, 1));

        if (PQgetisnull(res, i, 2))
        {
            xmlNewChild(q_node, NULL, BAD_CAST "stuAnswer", BAD_CAST "");
        } else {
            xmlNewChild(q_node, NULL, BAD_CAST "stuAnswer", BAD_CAST PQgetvalue(res, i, 2));
        }
        
    }

    PQclear(res);

    xmlChar *buffer;
    int buffersize;

    xmlDocDumpFormatMemory(doc, &buffer, &buffersize, 1);

    response = sys_error(PC_SUCCESSFUL);
    response += "\r\n\r\n";
    response += (char *) buffer;

    xmlFreeDoc(doc);
    xmlFree(buffer);

    return response;

}
