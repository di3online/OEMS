/**
 * @file setExamInfo.cpp
 * @brief
 * @author ZhuZY
 * @version 1.4
 * @date 2012-05-04
 */
#include "exam.h"
#include "handlers.h"
using namespace std;

/**
 * @brief handle_MEINF
 *
 * @param rawtext
 *          The text which contain cookie, eid and the start time
 * @param userID
 *          The uid
 * @param eid
 *          The eid
 * @param exam_name
 *          The name of exam
 * @param start_time
 *          The time of start
 * @param end_time
 *          The time of end
 * @param accomplishment
 *          is the teacher accomplishment the exam
 * @param management
 *          online or not
 * @param status
 *          the teacher check the exam shatus, closed/started/finished
 * @param course
 *          the course of the exam
 */
string handle_MEINF(const string &rawtext)
{
    uid_t userID;
    eid_t eid;
    string cookie = "";
    string exam_name = "";
    string start_time = "";
    string end_time = "";
    string accomplishment = "false";
    string management = "offline";
    string status = "";
    string course = "";

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

    //Get the exam_name
    start = rawtext.find("<name>", end) + 6;
    end = rawtext.find("</name>", start);
    exam_name = rawtext.substr(start, end - start);

    //Get the course
    start = rawtext.find("<course>", end) + 8;
    end = rawtext.find("</course>", start);
    course = rawtext.substr(start, end - start);

    //Get the start_time
    start = rawtext.find("<stime>", end) + 7;
    end = rawtext.find("</stime>", start);
    start_time = rawtext.substr(start, end - start);

    //Get the end_time
    start = rawtext.find("<etime>", end) + 7;
    end = rawtext.find("</etime>", start);
    end_time = rawtext.substr(start, end - start);

    //Get the management
    start = rawtext.find("<management>", end) + 12;
    end = rawtext.find("</management>", start);
    management = rawtext.substr(start, end - start);

    //Get the status
    start = rawtext.find("<status>", end) + 8;
    end = rawtext.find("</status>", start);
    status = rawtext.substr(start, end - start);

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

    setExamInfo(userID, eid, exam_name, start_time, end_time, management, status,
                course, err, db.getConn());

    if (err != PC_SUCCESSFUL)
    {
        response = sys_error(err);
        response += "\r\n\r\n";
        PQexec(db.getConn(), "ROLLBACK");
        return response;
    }

    dbres = PQexec(db.getConn(), "COMMIT");

    response = sys_error(PC_SUCCESSFUL);
    response += "\r\n\r\n";

    return response;
}
