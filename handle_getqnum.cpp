#include "handlers.h"

#include <sstream>

/**
 * @brief handle_GETQNUM
 *
 * @param rawtext
 *          The text which contain cookie, eid and the start time
 * @param pid
 *          The paper_id
 */
string handle_GETQNUM(const string &rawtext)
{
    pid_t pid;
    string response = "";
    string qnum = "";

    int err = PC_UNKNOWNERROR;
    DB db;

    size_t start = rawtext.find(' ') + 1;
    size_t end = 0;

    //Get the pid
    end = rawtext.find("\r\n", start);
    pid = rawtext.substr(start, end - start);

    PGresult *dbres = PQexec(db.getConn(), "BEGIN");
    if (PQresultStatus(dbres) != PGRES_COMMAND_OK)
    {
        response = sys_error(PC_DBERROR);
        response += "\r\n\r\n";
        PQclear(dbres);
        return response;
    }
    PQclear(dbres);

    char sql[300];
    size_t number_pid;
    stringstream ss;

    ss << pid;
    ss >> number_pid;
    snprintf(sql, sizeof(sql), "SELECT count(*) FROM question WHERE paper_id = %lu", number_pid);
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
    //check if the pid is exist
    if( PQntuples(res) == 0)
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

    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node1 = xmlNewNode(NULL, BAD_CAST "GETQNUM");
    xmlDocSetRootElement(doc, root_node1);
    qnum = PQgetvalue(res, 0, 0);
    xmlNewChild(root_node1, NULL, BAD_CAST "qnum", BAD_CAST qnum.c_str());

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


