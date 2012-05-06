/**
 * @file question.cpp
 * @brief
 * @author Qian Zhigang
 * @version 1.2
 * @date 2012-04-30
 */
#include "question.h"

/**
 * @brief addChoiceToQuestion
 *          add the Choice
 *          to Question and return the question if succeed
 * @param userID
 *          User's ID which will be checked
 * @param questionID
 *          question's ID which will be checked
 * @param choiceID
 *          choice's ID which will be checked
 * @param err
 *          return the status of the function progress
 * @param dbconn
 *          the connection to database which is created in
 *          the caller.
 *
 * @return
 *          The string which contain err = 0
 *          if it is correct
 */

void addChoiceToQuestion(const uid_t &userID,const qid_t &questionID,const cid_t &choiceID,
                            int &err, PGconn *dbconn)
{
    err = PC_UNKNOWNERROR;
    PGconn *conn = dbconn;

    string ret = "";

    if (PQstatus(conn) != CONNECTION_OK)
    {
        err = PC_DBERROR;
        return ;
    }

    //check the Permissions
    int check_GID = getGIDByUID(userID,err,dbconn);
    if (err != PC_SUCCESSFUL)
    {
        err = PC_DBERROR;
        return ;
    }

    if (check_GID == GID_ADMIN ||check_GID == GID_TEACHER)
    {

        //escapte string of userID and password
        char ccid[2 * choiceID.size() + 1];
        char cqid[2 * questionID.size() + 1];

        PQescapeString(ccid, choiceID.c_str(), choiceID.size());
        PQescapeString(cqid, questionID.c_str(), questionID.size());

        char sql[300];
        snprintf(sql, sizeof(sql), "SELECT question_id FROM choice where choice_id = '%s' ",ccid);
        //Exec the SQL query
        PGresult *res = PQexec(conn, sql);
        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            err = PC_DBERROR;
            PQclear(res);
            return ;
            // ret += PQresStatus(PQresultStatus(res));
            // ret += PQerrorMessage(conn);
        }

        if (PQntuples(res) == 0)
        {//If return null
            err = PC_NOTFOUND;
            PQclear(res);
            return ;
        }

        PQclear(res);

        snprintf(sql, sizeof(sql), "UPDATE choice SET question_id = '%s' where(choice_id = '%s' ) ",cqid,ccid);

        //Exec the SQL query
        res = PQexec(conn, sql);

        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {//If exection failed
            err = PC_DBERROR;
            PQclear(res);
            return ;
        }

        PQclear(res);

        err = PC_SUCCESSFUL;

        return ;

    }else
    {
        err = PC_NOPERMISSION;
        return ;
    }


}


/**
 * @brief creatQuestion
 *          Check the userID
 *          and return the question if succeed
 * @param userID
 *          User's ID which will be checked
 * @param err
 *          return the status of the function progress
 * @param dbconn
 *          the connection to database which is created in
 *          the caller.
 * @return
 *          The string which contain question
 *          if userID is correct
 */

string createQuestion(const uid_t &userID,int &err, PGconn *dbconn)
{
    err = PC_UNKNOWNERROR;
    PGconn *conn = dbconn;

    string ret = "";

    if (PQstatus(conn) != CONNECTION_OK)
    {
        err = PC_DBERROR;
        ret = "";
        return ret;
    }

    //check the Permissions
    int check_GID = getGIDByUID(userID,err,dbconn);
    if (err != PC_SUCCESSFUL)
    {
        err = PC_DBERROR;
        ret = "";
        return ret;
    }

    if (check_GID == GID_ADMIN ||check_GID == GID_TEACHER)
    {
        char sql[300];
        snprintf(sql, sizeof(sql), "INSERT INTO question VALUES (DEFAULT) RETURNING question_id");

        //Exec the SQL query
        PGresult *res = PQexec(conn, sql);

        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {//If exection failed
            //ret += PQresStatus(PQresultStatus(res));
            //ret += PQerrorMessage(conn);
            err = PC_DBERROR;
            ret = "";
            PQclear(res);
            return ret;
        }

        string temp_id = "";
        temp_id = PQgetvalue(res,0,0);
        ret = temp_id;

        PQclear(res);

        //set the err
        err = PC_SUCCESSFUL;

        return ret;

    }else
    {
        err = PC_NOPERMISSION;
        return ret;
    }

}

/**
 * @brief deleteChoiceFromQuestion
 *          delete the Choice
 *          to Question and return the question if succeed
 * @param userID
 *          User's ID which will be checked
 * @param questionID
 *          question's ID which will be checked
 * @param choiceID
 *          choice's ID which will be checked
 * @param err
 *          return the status of the function progress
 * @param dbconn
 *          the connection to database which is created in
 *          the caller.
 * @return
 *          The string which contain 200 OK
 *          if it is correct
 */

void deleteChoiceFromQuestion(const uid_t &userID,const qid_t &questionID,const cid_t &choiceID,
                               int &err, PGconn *dbconn)
{
    err = PC_UNKNOWNERROR;
    PGconn *conn = dbconn;

    string ret = "";

    if (PQstatus(conn) != CONNECTION_OK)
    {
        err = PC_DBERROR;
        return ;
    }

    //check the Permissions
    int check_GID = getGIDByUID(userID,err,dbconn);
    if (err != PC_SUCCESSFUL)
    {
        err = PC_DBERROR;
        return ;
    }

    if (check_GID == GID_ADMIN ||check_GID == GID_TEACHER)
    {
        //escapte string of userID and password
        char ccid[2 * choiceID.size() + 1];

        PQescapeString(ccid, choiceID.c_str(), choiceID.size());

        char sql[300];
        snprintf(sql, sizeof(sql), "SELECT question_id FROM choice where choice_id = '%s' ",ccid);
        //Exec the SQL query
        PGresult *res = PQexec(conn, sql);
        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            //ret += PQresStatus(PQresultStatus(res));
            //ret += PQerrorMessage(conn);
            err = PC_DBERROR;
            PQclear(res);
            return ;
        }

        if (PQntuples(res) == 0)
        {//If return null
            err = PC_NOTFOUND;
            PQclear(res);
            return ;
        }

        PQclear(res);

        snprintf(sql, sizeof(sql), "DELETE  FROM choice WHERE choice_id = '%s'",ccid);

        //Exec the SQL query
        res = PQexec(conn, sql);

        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {//If exection failed
            err = PC_DBERROR;
            PQclear(res);
            return ;
        }

        PQclear(res);
        err = PC_SUCCESSFUL;
        return ;

    }else
    {
        err = PC_NOPERMISSION;
        return ;
    }

}


/**
 * @brief deleteQuestion
 *          Check the userID
 *          and delete the question if succeed
 * @param userID
 *          User's ID which will be checked
 * @param questionID
 *          question's ID which will be checked
 * @param err
 *          return the status of the function progress
 * @param dbconn
 *          the connection to database which is created in
 *          the caller.
 * @return
 *          The string which contain question
 *          if userID is correct
 */

void deleteQuestion(const uid_t &userID,const qid_t &questionID,
                         int &err, PGconn *dbconn)
{
    err = PC_UNKNOWNERROR;
    PGconn *conn = dbconn;

    string ret = "";

    if (PQstatus(conn) != CONNECTION_OK)
    {
        err = PC_DBERROR;
        return ;
    }

    //check the Permissions
    int check_GID = getGIDByUID(userID,err,dbconn);
    if (err != PC_SUCCESSFUL)
    {
        err = PC_DBERROR;
        return ;
    }

    if (check_GID == GID_ADMIN ||check_GID == GID_TEACHER)
    {
        //escapte string of userID and password
        char cqid[2 * questionID.size() + 1];

        PQescapeString(cqid, questionID.c_str(), questionID.size());

        char sql[300];
        // make sure the question_id is exist.
        snprintf(sql, sizeof(sql), "DELETE FROM question where question_id = '%s' ",cqid);
        //Exec the SQL query
        PGresult *res = PQexec(conn, sql);
        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            //ret += PQresStatus(PQresultStatus(res));
            //ret += PQerrorMessage(conn);
            err = PC_DBERROR;
            PQclear(res);
            return ;
        }

//        if (PQntuples(res) == 0)
//        {//If return null
//            err = PC_NOTFOUND;
//            PQclear(res);
//            return ;
//        }

        PQclear(res);

        err = PC_SUCCESSFUL;
        return ;

    }else
    {
        err = PC_NOPERMISSION;
        return ;
    }

}

/**
 * @brief setKeyToQuestion
 *          add the Choice
 *          to Question and return the question if succeed
 * @param userID
 *          User's ID which will be checked
 * @param questionID
 *          question's ID which will be checked
 * @param choiceID
 *          choice's ID which will be checked
 * @param err
 *          return the status of the function progress
 * @param dbconn
 *          the connection to database which is created in
 *          the caller.
 * @return
 *          The string which contain 200 OK
 *          if it is correct
 */

void setKeyToQuestion(const uid_t &userID,const qid_t &questionID,const cid_t &choiceID,
                       int &err, PGconn *dbconn)
{
    err = PC_UNKNOWNERROR;
    PGconn *conn = dbconn;

    string ret = "";

    if (PQstatus(conn) != CONNECTION_OK)
    {
        err = PC_DBERROR;
        return ;
    }

    //check the Permissions
    int check_GID = getGIDByUID(userID,err,dbconn);
    if (err != PC_SUCCESSFUL)
    {
        err = PC_DBERROR;
        return ;
    }

     if (check_GID == GID_ADMIN ||check_GID == GID_TEACHER)
     {
        //escapte string of userID and password
        char ccid[2 * choiceID.size() + 1];
        char cqid[2 * questionID.size() + 1];

        PQescapeString(ccid, choiceID.c_str(), choiceID.size());
        PQescapeString(cqid, questionID.c_str(), questionID.size());

        char sql[300];
        snprintf(sql, sizeof(sql), "SELECT paper_id FROM question where question_id = '%s' ",cqid);
        //Exec the SQL query
        PGresult *res = PQexec(conn, sql);
        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            //ret += PQresStatus(PQresultStatus(res));
            //ret += PQerrorMessage(conn);
            err = PC_DBERROR;
            PQclear(res);
            return ;
        }

        if (PQntuples(res) == 0)
        {//If return null
            err = PC_NOTFOUND;
            PQclear(res);
            return ;
        }

        PQclear(res);
        snprintf(sql, sizeof(sql), "UPDATE question SET key = '%s' where(question_id = '%s' ) ",ccid,cqid);

        //Exec the SQL query
        res = PQexec(conn, sql);

        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {//If exection failed
            // ret += PQresStatus(PQresultStatus(res));
            // ret += PQerrorMessage(conn);
            err = PC_DBERROR;
            PQclear(res);
            return ;
        }

        PQclear(res);
        err = PC_SUCCESSFUL;
        return ;

    }else
    {
        err = PC_NOPERMISSION;
        return ;
    }

}

/**
 * @brief setQuestionDescription
 *          modify the question description
 *          and return 200 ok if succeed
 * @param userID
 *          User's ID which will be checked
 * @param questionID
 *          question's ID which will be checked
 * @param description
 *          the question description.
 * @param err
 *          return the status of the function progress
 * @param dbconn
 *          the connection to database which is created in
 *          the caller.
 * @return
 *          The string which contain 200 OK
 *          if it is correct
 */

void setQuestionDescription(const uid_t &userID,const qid_t &questionID,const std::string description,
                             int &err, PGconn *dbconn)
{
    err = PC_UNKNOWNERROR;
    PGconn *conn = dbconn;

    string ret = "";

    if (PQstatus(conn) != CONNECTION_OK)
    {
        err = PC_DBERROR;
        return ;
    }

    //check the Permissions
    int check_GID = getGIDByUID(userID,err,dbconn);
    if (err != PC_SUCCESSFUL)
    {
        err = PC_DBERROR;
        return ;
    }

    if (check_GID == GID_ADMIN ||check_GID == GID_TEACHER)
    {
        //escapte string of userID and password
        //char cuid[2 * userID.size() + 1];
        char cqid[2 * questionID.size() + 1];
        char cdescription[2 * description.size() + 1];

        //PQescapeString(cuid, userID.c_str(), userID.size());
        PQescapeString(cqid, questionID.c_str(), questionID.size());
        PQescapeString(cdescription, description.c_str(), description.size());

        char sql[300];
        //check the question_id is exist
        snprintf(sql, sizeof(sql), "SELECT paper_id FROM question where question_id = '%s' ",cqid);
        //Exec the SQL query
        PGresult *res = PQexec(conn, sql);
        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            //ret += PQresStatus(PQresultStatus(res));
            //ret += PQerrorMessage(conn);
            err = PC_DBERROR;
            PQclear(res);
            return ;
        }

        if (PQntuples(res) == 0)
        {//If return null
            err = PC_NOTFOUND;
            PQclear(res);
            return ;
        }

        PQclear(res);

        snprintf(sql, sizeof(sql), "UPDATE question SET content = '%s' WHERE question_id = '%s'",cdescription,cqid);

        //Exec the SQL query
        res = PQexec(conn, sql);

        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {   //If exection failed
            // ret += PQresStatus(PQresultStatus(res));
            // ret += PQerrorMessage(conn);
            err = PC_DBERROR;
            PQclear(res);
            return ;
        }

        PQclear(res);
        err = PC_SUCCESSFUL;
        return ;

    }else
    {
        err = PC_NOPERMISSION;
        return ;
    }

}


