#include "exam.h"
#include "paper.h"
#include "handlers.h"

/**
 * @brief handle_ADDE
 * @author ZhuZY
 * @param rawtext
 *          The text which contain cookie
 */
string handle_ADDE(const string &rawtext)
{
    uid_t userID;
    string cookie = "";
    string exam_name = "";
    string start_time = "";
    string end_time = "";
    string management = "offline";
    string status = "closed";
    string course = "";

    string response = "";

    int err = PC_UNKNOWNERROR;
    DB db;

    size_t start = rawtext.find("Cookie: ") + 8;
    size_t end = 0;

    //Get the cookie
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

    eid_t eid = createExam(userID, err, db.getConn());
    if (err != PC_SUCCESSFUL)
    {
        response = sys_error(err);
        response += "\r\n\r\n";
        PQexec(db.getConn(), "ROLLBACK");
        return response;
    }

    pid_t paper1 = createPaper(userID, err, db.getConn());
    pid_t paper2 = createPaper(userID, err, db.getConn());
    pid_t paper3 = createPaper(userID, err, db.getConn());
    if (err != PC_SUCCESSFUL)
    {
        response = sys_error(err);
        response += "\r\n\r\n";
        PQexec(db.getConn(), "ROLLBACK");
        return response;
    }

    addPaperToExam(userID, paper1, eid, err, db.getConn());
    addPaperToExam(userID, paper2, eid, err, db.getConn());
    addPaperToExam(userID, paper3, eid, err, db.getConn());
    if (err != PC_SUCCESSFUL)
    {
        response = sys_error(err);
        response += "\r\n\r\n";
        PQexec(db.getConn(), "ROLLBACK");
        return response;
    }

    setExamInfo(userID, eid, exam_name, start_time, end_time, management, status,
                course, err, db.getConn());
    //Check the return code of the function
    if (err != PC_SUCCESSFUL)
    {
        response = sys_error(err);
        response += "\r\n\r\n";
        PQexec(db.getConn(), "ROLLBACK");
        return response;
    }
    dbres = PQexec(db.getConn(), "COMMIT");
    PQclear(dbres);

    response = sys_error(PC_SUCCESSFUL);
    response += "\r\n\r\n";

    return response;
}
