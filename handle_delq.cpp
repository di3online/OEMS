#include "question.h"

string handle_DELQ(const string &rawtext)
{
    qid_t questionID;
    string cookie;

    size_t start = 0;
    size_t end = rawtext.find(' ');

    start = end+1;
    end = rawtext.find("\r\n",start);

    questionID = rawtext.substr(start,end-start);

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

    deleteQuestion(userID, questionID, err, db.getConn());
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



