#include "exam.h"

string handle_DELE(const string &rawtext)
{
    eid_t examID;
    string cookie;

    size_t start = 0;
    size_t end = rawtext.find(' ');

    start = end+1;
    end = rawtext.find("\r\n",start);

    examID = rawtext.substr(start,end-start);

    start = end + 1;
    end = rawtext.find(' ',start);

    start = end+1;
    end = rawtext.find("\r\n",start);

    cookie = rawtext.substr(start,end-start);

    int err;
    DB db;
    string response;

    uid_t userID = getUIDByCookie(cookie,err,db.getConn());
    if (err != PC_SUCCESSFUL)
    {
        response = sys_error(err);
        response += "\r\n\r\n";
        return response;
    }

    deleteExam(userID, examID, err, db.getConn());
    if (err != PC_SUCCESSFUL)
    {
        response = sys_error(err);
        response += "\r\n\r\n";
        return response;
    }

    response = sys_error(PC_SUCCESSFUL);
    response += "\r\n\r\n";
    return response;

}

