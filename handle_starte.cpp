#include "common.h"
#include "db.h"
#include "xml.h"

#include "score.h"

#include <cstring>
#include <vector>

using namespace std;

    string 
handle_STARTE(const string &rawtext)
{
    int err;
    string ret;

    const char *handlefind = "STAERE ";
    const char *strfind = "\r\nCookie: ";

    size_t start = rawtext.find(handlefind) + strlen(handlefind);

    if (start == string::npos)
    {
        err = PC_INPUTFORMATERROR;
        ret = sys_error(err);
        ret += "\r\n\r\n";
        return ret;
    }

    size_t stop = rawtext.find(strfind, start);

    if (stop == string::npos)
    {
        err = PC_INPUTFORMATERROR;
        ret = sys_error(err);
        ret += "\r\n\r\n";
        return ret;
    }

    eid_t eid = rawtext.substr(start, stop - start);

    start = stop + strlen(strfind);

    stop = rawtext.find("\r\n", start);

    if (stop == string::npos)
    {
        err = PC_INPUTFORMATERROR;
        ret = sys_error(err);
        ret += "\r\n\r\n";
        return ret;
    }

    string cookie = rawtext.substr(start, stop - start);

    DB db;
    PGconn *dbconn = db.getConn();

    uid_t uid = getUIDByCookie(cookie, err, dbconn);

    gid_t gid = getGIDByUID(uid, err, dbconn);

    if (GID_STUDENT == gid 
            || gid == GID_ADMIN || gid == GID_TEACHER)
    {
        char ceid[MAXBUF];
        PQescapeString(ceid, eid.c_str(), eid.size() + 1);

        char sql_buf[MAXBUF];
        PGresult *res_exam;
        snprintf(sql_buf, sizeof(sql_buf), 
                "SELECT start_time as start, end_time, status as end "
                " FROM exam WHERE exam_id=%s "
                    "AND status = 'started'",
                ceid);

        res_exam = PQexec(dbconn, sql_buf);

        if (PQresultStatus(res_exam) != PGRES_TUPLES_OK)
        {
            err = PC_DBERROR;
            ret = sys_error(err);
            ret += "\r\n\r\n";
            PQclear(res_exam);
            return ret;
        }

        if (PQntuples(res_exam) == 0)
        {
            err = PC_NOTFOUND;
            ret = sys_error(err);
            ret += "\r\n\r\n";
            PQclear(res_exam);
            return ret;
        }

        // Is exam started
        time_t start = strtoul(PQgetvalue(res_exam, 0, 0), NULL, 10);
        time_t end = strtoul(PQgetvalue(res_exam, 0, 1), NULL, 10);

#if DBG
        fprintf(stderr, "%s:%d::start: %ul, end: %ul", __FUNCTION__,
                __LINE__, start, end)

#endif

            time_t now = time(NULL);
        if ( ! (now >=start 
                    && now <= end 
                    && strcmp("started", PQgetvalue(res_exam, 0, 2) ) == 0) )
        {
            err = PC_NOPERMISSION;
            ret = sys_error(err);
            ret += "\r\n\r\n";
            PQclear(res_exam);
            return ret;
        }

        PQclear(res_exam);

        snprintf(sql_buf, sizeof(sql_buf), 
                "SELECT * FROM student_exam WHERE user_id = '%s' AND paper_id IN ( "
                    "SELECT paper_id FROM paper WHERE exam_id = %s "
                ") AND final_score IS NOT NULL ",
                uid.c_str(), ceid);

        res_exam = PQexec(dbconn, sql_buf);

        if (PQresultStatus(res_exam) != PGRES_TUPLES_OK)
        {
            err = PC_DBERROR;
            ret = sys_error(err);
            ret += "\r\n\r\n";

            std::cerr << PQerrorMessage(dbconn) << std::endl;
            std::cerr.flush();
            PQclear(res_exam);
            return ret;
        }
        
        if (PQntuples(res_exam) >= 1)
        {
            err = PC_NOPERMISSION;
            ret = sys_error(err);
            ret += "\r\n\r\n";

            std::cerr << "Student can't take part in the same exam twice. " << std::endl;
            std::cerr.flush();

            PQclear(res_exam);
            return ret;
        }

        PQclear(res_exam);

        // time is valid, start the exam


        // check if we are crashed
        // TODO: check resume action

        // SELECT paper_id FROM paper WHERE exam_id=eid and paper_id=(
        //     SELECT currentpaper FROM users WHERE user_id=uid
        // );
        //

        snprintf(sql_buf, sizeof(sql_buf), 
                "SELECT paper_id FROM paper "
                "WHERE exam_id = %s "
                "AND paper_id = ( "
                "SELECT currentp FROM users WHERE user_id = '%s' "
                ")",
                ceid, uid.c_str());

        res_exam = PQexec(dbconn, sql_buf);

        if (PQresultStatus(res_exam) != PGRES_TUPLES_OK)
        {
            err = PC_DBERROR;
            ret = sys_error(err);
            ret += "\r\n\r\n";
            ret += PQerrorMessage(dbconn);
            PQclear(res_exam);
            return ret;
        }


        if (PQntuples(res_exam) > 0)
        {
            //This is a resume action, do nothing
            return "200 OK\r\n\r\n";
        }

        PQclear(res_exam);

        // TODO: brand new paper, do init

        // It's SO ugly! Use std::vector to
        // store all the PGresult and free it
        // before return
        int number_of_paper = 0;
        snprintf(sql_buf, sizeof(sql_buf), 
                "SELECT paper_id FROM paper "
                "WHERE exam_id = '%s' ", ceid);
        PGresult *res = PQexec(dbconn, sql_buf);

        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            err = PC_DBERROR;
            ret = sys_error(err);
            ret += "\r\n\r\n";

            ret += PQerrorMessage(dbconn);
            //ret += PQerrorMessage(dbconn);
            PQclear(res);
            //     PQclear(res_paper);

            return ret;
        }

        vector<pid_t> plist;
        for (int i = 0; i < PQntuples(res); i++)
        {
            plist.push_back(PQgetvalue(res, i, 0));
        }

        number_of_paper = plist.size();

        PQclear(res);

        srand(now);
        pid_t currentp = plist[rand() % number_of_paper];

        //TODO: choose a paper differ from the nearby
        // SELECT paper_id as pid FROM paper WHERE 
        //   pid NOT IN (
        //     SELECT currentp FROM users WHERE userid = uid+1
        //       OR userid=uid-1
        // );

        // choose a sequence scheme

        snprintf(sql_buf, sizeof(sql_buf), 
                "SELECT count(*) FROM question WHERE paper_id = %s", 
                currentp.c_str());
        res = PQexec(dbconn, sql_buf);

        if (PQresultStatus(res) != PGRES_TUPLES_OK && PQntuples(res) == 0)
        {
            err = PC_DBERROR;
            ret = sys_error(err);
            ret += "\r\n\r\n";

            ret += PQerrorMessage(dbconn);
            PQclear(res);
            return ret;
        }

        int number_of_question = atoi(PQgetvalue(res, 0, 0));


        srand(now);
        int scheme = rand() % number_of_question;
        PQclear(res);
        //TODO: put scheme into DB
        // TODO: put the paper_id into DB
        // UPDATE users SET currentp=pid, scheme=scheme
        //  WHERE userid=uid;
        snprintf(sql_buf, sizeof(sql_buf), 
                "UPDATE users SET currentp = %s, scheme = %d, qflag = 0"
                "WHERE user_id = '%s'",
                currentp.c_str(), scheme, uid.c_str());

        res = PQexec(dbconn, sql_buf);

        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            err = PC_DBERROR;
            ret = sys_error(err);
            ret += "\r\n\r\n";

            ret += PQerrorMessage(dbconn);
            PQclear(res);
            return ret;
        }

        PQclear(res);

        snprintf(sql_buf, sizeof(sql_buf), 
                "INSERT INTO student_exam VALUES ("
                "'%s', %s, false, null)",
                uid.c_str(), currentp.c_str());

        res = PQexec(dbconn, sql_buf);

        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            err = PC_DBERROR;
            ret = sys_error(err);
            ret += "\r\n\r\n";

            ret += PQerrorMessage(dbconn);
            PQclear(res);
            return ret;
        }

        err = PC_SUCCESSFUL;
        ret = sys_error(err);
        ret += "\r\n\r\n";

        return ret;
    } else {
        err = PC_NOPERMISSION;
        ret = sys_error(err);
        ret += "\r\n\r\n";
        return ret;
    }
}


// qlist: the question list
// scheme: the sequence scheme between 0 and qlist.size() - 1
// current: number of question have done yet
    static qid_t 
getNextQ(std::vector<qid_t> &qlist, int scheme, int current)
{
    int flag = (scheme + current) % qlist.size();
    return qlist[flag];
}


    string 
handle_NEXTQ(const string &rawtext)
{
    int err;
    string ret;

    const char *handlefind = "NEXTQ ";
    const char *strfind = "\r\nCookie: ";

    size_t start = rawtext.find(handlefind) + strlen(handlefind);

    if (start == string::npos)
    {
        err = PC_INPUTFORMATERROR;
        ret = sys_error(err);
        ret += "\r\n\r\n";
        return ret;
    }

    size_t stop = rawtext.find(strfind, start);

    if (stop == string::npos)
    {
        err = PC_INPUTFORMATERROR;
        ret = sys_error(err);
        ret += "\r\n\r\n";
        return ret;
    }

    eid_t eid = rawtext.substr(start, stop - start);

    start = stop + strlen(strfind);

    stop = rawtext.find("\r\n", start);

    if (stop == string::npos)
    {
        err = PC_INPUTFORMATERROR;
        ret = sys_error(err);
        ret += "\r\n\r\n";
        return ret;
    }

    string cookie = rawtext.substr(start, stop - start);

    DB db;
    PGconn *dbconn = db.getConn();

    char sql_buf[MAXBUF];

    uid_t uid = getUIDByCookie(cookie, err, dbconn);

    snprintf(sql_buf, sizeof(sql_buf),
            "SELECT question_id FROM question WHERE "
            "paper_id = ( "
                "SELECT currentp FROM users WHERE "
                "user_id = '%s'"
            ") ORDER BY question_id ",
            uid.c_str());

    PGresult *res = PQexec(dbconn, sql_buf);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        err = PC_DBERROR;
        ret = sys_error(err);
        ret += "\r\n\r\n";
        ret += PQerrorMessage(dbconn);
        PQclear(res);
        return ret;
    }
    
    vector<qid_t> qlist;
    qlist.reserve(PQntuples(res));

    for (int i = 0; i < PQntuples(res); ++i)
    {
        qlist.push_back(PQgetvalue(res, i, 0));
    }
    
    PQclear(res);

    snprintf(sql_buf, sizeof(sql_buf),
            "SELECT scheme, qflag, currentp FROM users "
            "WHERE user_id = '%s' ",
            uid.c_str());

    res = PQexec(dbconn, sql_buf);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
//            || PQgetisnull(res, 0, 0) 
//            || PQgetisnull(res, 0, 1)
//            || PQgetisnull(res, 0, 2))
    {
        err = PC_DBERROR;
        ret = sys_error(err);
        ret += "\r\n\r\n";
        ret += PQerrorMessage(dbconn);
        PQclear(res);
        return ret;
    }

    int scheme = atoi(PQgetvalue(res, 0, 0));

    //qflag
    unsigned int current = atoi(PQgetvalue(res, 0, 1));

    //current paper
    pid_t pid = PQgetvalue(res, 0, 2);

    PQclear(res);

    if (current >= qlist.size())
    {


        //TODO: Call judge 
        //TODO: Remove the flag From user;

        snprintf(sql_buf, sizeof(sql_buf), 
                "UPDATE users SET scheme = null, qflag = null, currentp = null "
                "WHERE user_id = '%s'",
                uid.c_str());

        res = PQexec(dbconn, sql_buf);

        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            err = PC_DBERROR;
            ret = sys_error(err);
            ret += "\r\n\r\n";
            ret += PQerrorMessage(dbconn);

            PQclear(res);
            return ret;
        }

        PQclear(res);

        getScore(uid, pid, err, dbconn);

        if (err != PC_SUCCESSFUL)
        {
            ret = sys_error(err);
            ret += "\r\n\r\n";

            ret += PQerrorMessage(dbconn);
            return ret;
        }

        //Exam is over
        err = PC_EXAMOVER;
        ret = sys_error(err);
        ret += "\r\n\r\n";

        return ret;
    }

    // now get the pid!
    qid_t qidnum = getNextQ(qlist, scheme, current);

    snprintf(sql_buf, sizeof(sql_buf),
            "SELECT content, choice_id, answer_content, timelimit FROM question, choice "
            "WHERE question.question_id = %s "
            "AND choice.question_id = question.question_id ",
            qidnum.c_str());

    res = PQexec(dbconn, sql_buf);
    if (PQresultStatus(res) != PGRES_TUPLES_OK 
            || PQntuples(res) == 0)
    {
        err = PC_DBERROR;
        ret = sys_error(err);
        ret += "\r\n\r\n";

        ret += PQerrorMessage(dbconn);
        PQclear(res);
        return ret;
    }


    // now that we've got the pid, generate the paper
    // TODO: generate the XML tree
    
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");

    xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "NEXTQ");
    
    xmlDocSetRootElement(doc, root_node);

    xmlNewChild(root_node, NULL, BAD_CAST "qid", BAD_CAST qidnum.c_str());

    xmlNewChild(root_node, NULL, BAD_CAST "description", BAD_CAST PQgetvalue(res, 0, 0));

    xmlNewChild(root_node, NULL, BAD_CAST "time", BAD_CAST PQgetvalue(res, 0, 3));

    for (int i = 0; i < PQntuples(res); ++i)
    {
        xmlNodePtr c_node = xmlNewNode(NULL, BAD_CAST "choice");
        xmlAddChild(root_node, c_node);
        xmlNewChild(c_node, NULL, BAD_CAST "cid", BAD_CAST PQgetvalue(res, i, 1));
        xmlNewChild(c_node, NULL, BAD_CAST "description", BAD_CAST PQgetvalue(res, i, 2));
    }

    snprintf(sql_buf, sizeof(sql_buf),
            "%d", current);

    xmlNewChild(root_node, NULL, BAD_CAST "fnum", BAD_CAST sql_buf);

    snprintf(sql_buf, sizeof(sql_buf),
            "%lu", qlist.size());
    xmlNewChild(root_node, NULL, BAD_CAST "tnum", BAD_CAST sql_buf);

    xmlChar *xmlbuffer;
    int buffersize;

    xmlDocDumpFormatMemory(doc, &xmlbuffer, &buffersize, 1);

    err = PC_SUCCESSFUL;
    ret = sys_error(err);
    ret += "\r\n\r\n";

    ret += (char *) xmlbuffer;
    xmlFreeDoc(doc);
    xmlFree(xmlbuffer);



    PQclear(res);


    return ret;
}

