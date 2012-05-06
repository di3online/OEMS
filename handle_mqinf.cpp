/**
 * @file handle_mqinf.cpp
 * @brief
 * @author Qian Zhigang
 * @version 1.0
 * @date 2012-05-4
 */

#include "handlers.h"
#include <iostream>
#include "xml.h"
#include <cstring>
#include "paper.h"
#include "question.h"
#include "choice.h"

using namespace std;
/**
 * @brief handle_mqinf
 *
 * @param rawtext
 *          The text contains cookie which can resolve userID,questionID and choiceID
 * @param userID
 *          The user ID,question ID and choice ID will return
 */
 string handle_MQINF(const std::string &rawtext)
 {
    uid_t userID;
    qid_t questionID;
    cid_t choiceID;
    pid_t paperID;
    string temp_get;
    // define the return value
    string response;
    //save the choice id
    vector<string> v;

    string description = "";
    string choice[4] = "";
    string key = "";
    string time;
    string check_MQINF = "";

    size_t start = 0;
    size_t end = rawtext.find(" ");

    check_MQINF = rawtext.substr(start, end - start);
    //check the method is MQINF
    if (check_MQINF != "MQINF")
    {
        return "you called a wrong method which is not MQINF!\n";
    }

    //Init the err
    int err = PC_UNKNOWNERROR;
    //Init the db connection
    DB db;
    PGconn *conn = db.getConn();

    // get the questionID
    start = end + 1;
    end = rawtext.find("\r\n", start);
    temp_get= rawtext.substr(start, end - start);
    questionID = temp_get;

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
        return response;
    }

    //get the xml
    start = end + 4;
    end = rawtext.find("</MQINF>", start);
    temp_get = rawtext.substr(start, end - start + 8);
    //turn the string to char*
    int buf_size = temp_get.length();
    char buffer[buf_size];
    for (int i = 0; i <= buf_size;i++)
        buffer[i] = temp_get[i];

    //creat the xml tree
    xmlDocPtr pdoc = xmlParseMemory(buffer, buf_size);
    if( pdoc == NULL)
    {
     printf("Fail to parse XML buffer.\n");
    }
    xmlNodePtr root = xmlDocGetRootElement(pdoc);
    xmlNodePtr curr = root;

    //get the description
    while ( xmlStrcmp(curr->name, (const xmlChar *) "description") )
            curr = curr->xmlChildrenNode;
    char temp_arr[200];
    int temp_size = strlen((const char*)xmlNodeGetContent(curr->xmlChildrenNode)) + 1;
    strncpy(temp_arr, (const char*)xmlNodeGetContent(curr->xmlChildrenNode),temp_size);
    description = temp_arr;

    //get the choice
    int count_c = 0;
    for ( curr = curr->next; curr; curr = curr->next)
    {
        if ( xmlStrcmp(curr->name, (const xmlChar *) "choice") )
                break;

            xmlNodePtr data;
            data = curr->xmlChildrenNode;
            temp_size = strlen((const char*)xmlNodeGetContent(data)) + 1;
            strncpy(temp_arr, (const char*)xmlNodeGetContent(data),temp_size);
            choice[count_c] = temp_arr;
            count_c++;
    }

     //get the key
    temp_size = strlen((const char*)xmlNodeGetContent(curr->xmlChildrenNode)) + 1;
    strncpy(temp_arr, (const char*)xmlNodeGetContent(curr->xmlChildrenNode),temp_size);
    key = temp_arr;

    //get the time
    curr = curr->next;
    temp_size = strlen((const char*)xmlNodeGetContent(curr->xmlChildrenNode)) + 1;
    strncpy(temp_arr, (const char*)xmlNodeGetContent(curr->xmlChildrenNode),temp_size);
    string temp_str = temp_arr;
    time = temp_str;
    //std::istringstream in(temp_str);
    //in>>time;

    //begin to exec
    PGresult *res = PQexec(conn, "BEGIN");

    //check the db status
    if (PQstatus(conn) != CONNECTION_OK)
    {
        response = sys_error(err);
        PQclear(res);
        res = PQexec(conn, "ROLLBACK");
        PQclear(res);
        return response;
    }

     //check the Permissions
    int check_GID = getGIDByUID(userID,err,conn);
    if (err != PC_SUCCESSFUL)
    {
        response = sys_error(err);
        PQclear(res);
        res = PQexec(conn, "ROLLBACK");
        PQclear(res);
        return response;
    }
    if (check_GID == GID_ADMIN ||check_GID == GID_TEACHER)
    {
        // get the paperID
        char cqid[2 * questionID.size() + 1];
        PQescapeString(cqid, questionID.c_str(), questionID.size());
        char sql[300];
        snprintf(sql, sizeof(sql), "SELECT paper_id FROM question WHERE question_id = '%s'",cqid);
        //Exec the SQL query
        PGresult *res_1 = PQexec(conn, sql);
        if (PQresultStatus(res_1) != PGRES_TUPLES_OK)
        {
            //string ret = PQresStatus(PQresultStatus(res));
            //ret += PQerrorMessage(conn);
            //cout<<ret;
            err = PC_DBERROR;
            response = sys_error(err);
            PQclear(res_1);
            PQclear(res);
            res = PQexec(conn, "ROLLBACK");
            PQclear(res);
            return response;
        }
        paperID = PQgetvalue(res_1,0,0);
        PQclear(res_1);

        //delete the old question
        deleteQuestion(userID, questionID, err , conn);
        //Check the return code of the function
        if (err != PC_SUCCESSFUL)
        {
            response = sys_error(err);
            PQclear(res);
            res = PQexec(conn, "ROLLBACK");
            PQclear(res);
            return response;
        }

        //creat the question_id
        questionID = createQuestion(userID,err,conn);
        //Check the return code of the function
        if (err != PC_SUCCESSFUL)
        {
            response = sys_error(err);
            PQclear(res);
            res = PQexec(conn, "ROLLBACK");
            PQclear(res);
            return response;
        }

        //set the question description
        setQuestionDescription(userID, questionID, description,err,conn);
        //Check the return code of the function
        if (err != PC_SUCCESSFUL)
        {
            response = sys_error(err);
            PQclear(res);
            res = PQexec(conn, "ROLLBACK");
            PQclear(res);
            return response;
        }

        //get the chocie id
        for (int i = 0; i < count_c ; i++)
        {
            //create choice
            temp_str = createChoice(userID, err, conn);
            //Check the return code of the function
            if (err != PC_SUCCESSFUL)
            {
                response = sys_error(err);
                PQclear(res);
                res = PQexec(conn, "ROLLBACK");
                PQclear(res);
                return response;
            }
            v.push_back(temp_str);

            //create choice description
            setChoiceDescription (userID, v[i], choice[i], err, conn);
            if (err != PC_SUCCESSFUL)
            {
                response = sys_error(err);
                PQclear(res);
                res = PQexec(conn, "ROLLBACK");
                PQclear(res);
                return response;
            }

            addChoiceToQuestion(userID, questionID, v[i], err, conn);

            if (err != PC_SUCCESSFUL)
            {
                response = sys_error(err);
                PQclear(res);
                res = PQexec(conn, "ROLLBACK");
                PQclear(res);
                return response;

            }
        }

        //set the key choice
        int temp_x;
        std::istringstream in(key);
        in>>temp_x;
        temp_x -= 1;
        setKeyToQuestion(userID, questionID, v[temp_x], err, conn);
        if (err != PC_SUCCESSFUL)
        {
            response = sys_error(err);
            PQclear(res);
            res = PQexec(conn, "ROLLBACK");
            PQclear(res);
            return response;
        }

        //set the time
        //escapte string of time
        char ctime[2 * time.size() + 1];
        char cpid[2 * paperID.size() + 1];
        PQescapeString(cpid, paperID.c_str(), paperID.size());
        PQescapeString(ctime, time.c_str(), time.size());
        PQescapeString(cqid, questionID.c_str(), questionID.size());

        snprintf(sql, sizeof(sql), "UPDATE question SET timelimit = '%s' where question_id = '%s' ",ctime,cqid);
        //Exec the SQL query
        res_1 = PQexec(conn, sql);
        if (PQresultStatus(res_1) != PGRES_COMMAND_OK)
        {
            //string ret = PQresStatus(PQresultStatus(res));
            //ret += PQerrorMessage(conn);
            //cout<<ret;
            err = PC_DBERROR;
            response = sys_error(err);
            PQclear(res_1);
            PQclear(res);
            res = PQexec(conn, "ROLLBACK");
            PQclear(res);
            return response;
        }
        PQclear(res_1);

        //add question to paper
        addQuestionToPaper(userID, questionID, paperID, err, conn);
        if (err != PC_SUCCESSFUL)
            {
                response = sys_error(err);
                PQclear(res);
                res = PQexec(conn, "ROLLBACK");
                PQclear(res);
                return response;
            }

        //return the right result
        response = sys_error(PC_SUCCESSFUL);
        response += "\r\n\r\n";
        PQclear(res);
        res = PQexec(conn, "COMMIT");
        PQclear(res);
        return response;

    }else
    {
        err = PC_NOPERMISSION;
        response = sys_error(err);
        PQclear(res);
        res = PQexec(conn, "ROLLBACK");
        PQclear(res);
        return response;
    }

 }
