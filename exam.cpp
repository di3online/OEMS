/**
 * @file exam.cpp
 * @brief
 * @author BaiHan
 * @version 1.0
 * @date 2012-04-15
 */
#include "exam.h"

#include <sstream>
using namespace std;
/**
 * @brief addPaperToExam
 *          return successfully if user is a teacher or admin
 * @param userID
 *          User's ID which will be checked
 * @param err
 *          return the status of the function progress
 * @param dbconn
 *          the connection to database which is created in
 *          the caller.
 *
 * @return
 *          void
 */
void
addPaperToExam(const uid_t &uid, const pid_t &pid, const eid_t &eid,
	int &err, PGconn *dbconn)
{
    err = PC_UNKNOWNERROR;
    PGconn *conn = dbconn;

    if (PQstatus(conn) != CONNECTION_OK)
    {
        err = PC_DBERROR;
        return;
    }

    gid_t gid = getGIDByUID(uid, err, conn);

    if (err != PC_SUCCESSFUL)
    {
        //Don't Rollback here
        //PQexec(conn, "ROLLBACK");
        return;
    }

    //if (echo_gid(gid) != echo_gid(GID_ADMIN) && echo_gid(gid) != echo_gid(GID_TEACHER))
    //You can write in such way.
    if (gid != GID_ADMIN && gid != GID_TEACHER)
    {
        //If user is a student
        err = PC_NOPERMISSION;
        return;
        //PQclear(res);
    }

    char ceid[2 * eid.size() + 1];
    char cpid[2 * pid.size() + 1];

    PQescapeString(ceid, eid.c_str(), eid.size());
    PQescapeString(cpid, pid.c_str(), pid.size());


    char sql[300];
    snprintf(sql, sizeof(sql), "SELECT exam_id FROM exam WHERE exam_id='%s' ", ceid);


    //Exec the SQL query
    PGresult *res = NULL;
    res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {//If exection failed
        err = PC_DBERROR;
        PQclear(res);
        return;
    }

    if (PQntuples(res) == 0)
    {//If exam do not exist
        err = PC_NOTFOUND;
        PQclear(res);
        return;
    }

    //Don't forget clear the result.
    //Updated by: Lai
    PQclear(res);

    snprintf(sql, sizeof(sql), "SELECT paper_id FROM paper WHERE paper_id='%s' ", cpid);

    //Exec the SQL query
    res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {//If exection failed
        err = PC_DBERROR;
        PQclear(res);
        return;
    }

    if (PQntuples(res) == 0)
    {//If exam do not exist
        err = PC_NOTFOUND;
        PQclear(res);
        return;
    }

    //Don't forget clear the result.
    //Updated by: Lai
    PQclear(res);

    snprintf(sql, sizeof(sql), "UPDATE paper SET exam_id = '%s' WHERE paper_id = '%s' ", ceid, cpid);

    //Exec the SQL query
    res = PQexec(conn, sql);


    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {//If exection failed
        err = PC_DBERROR;
        PQclear(res);
        return;
    }


    err = PC_SUCCESSFUL;
    PQclear(res);

    return;
}


/**
 * @brief createExam
 *          return examID if user is a teacher or admin
 * @param userID
 *          User's ID which will be checked
 * @param err
 *          return the status of the function progress
 * @param dbconn
 *          the connection to database which is created in
 *          the caller.
 * @updated by ZhuZY
 * @return
 *          The string which contain exam_id
 *          if cookie is teacher or admin
 */

  eid_t createExam(const uid_t &uid, int &err, PGconn *dbconn)
{
    eid_t eid;
    err = PC_UNKNOWNERROR;
    PGconn *conn = dbconn;

    gid_t gid = getGIDByUID(uid, err, conn);

    if ((gid != GID_ADMIN) && (gid != GID_TEACHER))
    {
        err = PC_NOPERMISSION;
        return "PC_NOPERMISSION";
    }

    if (PQstatus(conn) != CONNECTION_OK)
    {
        err = PC_DBERROR;
    }

    char sql[300];

    snprintf(sql, sizeof(sql), "INSERT INTO exam(owner_id) VALUES('%s') RETURNING exam_id", uid.c_str());

    PGresult *res;
    //Exec the SQL query

    res = PQexec(conn, sql);
    if ((PQresultStatus(res) != PGRES_TUPLES_OK))
    {
        err = PC_DBERROR;
        PQclear(res);
        return "";
    }

    eid = PQgetvalue(res, 0, 0);
    PQclear(res);
    err = PC_SUCCESSFUL;

    return eid;
}


/**
 * @brief deleteExam
 *          return successfully if user is a teacher or admin
 * @param userID
 *          User's ID which will be checked
 * @param err
 *          return the status of the function progress
 * @param dbconn
 *          the connection to database which is created in
 *          the caller.
 *
 * @return
 *          void
 */

void
deleteExam(const uid_t &uid, const eid_t &eid, int &err, PGconn *dbconn)
{
	err = PC_UNKNOWNERROR;
    PGconn *conn = dbconn;

    if (PQstatus(conn) != CONNECTION_OK)
    {
        err = PC_DBERROR;
        return;
    }

    gid_t gid = getGIDByUID(uid, err, conn);

    if (err != PC_SUCCESSFUL)
    {
        PQexec(conn, "ROLLBACK");
        return;
    }

    if (gid != GID_ADMIN && gid != GID_TEACHER)
    {
        //If user is a student
        err = PC_NOPERMISSION;
        return;
        //PQclear(res);
    }

    //escapte string of eid
    char ceid[2 * eid.size() + 1];

    PQescapeString(ceid, eid.c_str(), eid.size());

    char sql[300];
    snprintf(sql, sizeof(sql), "SELECT exam_id FROM exam WHERE exam_id='%s' ", ceid);

    //Exec the SQL query
    PGresult *res = NULL;
    res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {//If exection failed
        err = PC_DBERROR;
        PQclear(res);
        return;
    }

    if (PQntuples(res) == 0)
    {//If exam do not exist
        err = PC_NOTFOUND;
        PQclear(res);
        return;
    }

    snprintf(sql, sizeof(sql), "DELETE FROM exam WHERE exam_id = '%s' ", ceid);
    res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {//If exection failed
        err = PC_DBERROR;
        PQclear(res);
        return;
    }

    err = PC_SUCCESSFUL;
    PQclear(res);
}


void setExamInfo(const uid_t &uid, const eid_t &eid,
                std::string exam_name, std::string start_time, std::string end_time,
                std::string management,std::string status, std::string course,
                int &err, PGconn *dbconn)
{
    string accomplishment = "false";

    err = PC_UNKNOWNERROR;
    PGconn *conn = dbconn;

    //check the group id
    gid_t gid = getGIDByUID(uid, err, conn);
    if ((gid != GID_ADMIN) && (gid != GID_TEACHER))
    {
        err = PC_NOPERMISSION;
        return;
    }

    if (PQstatus(conn) != CONNECTION_OK)
    {
        err = PC_DBERROR;
        return;
    }

    char sql[300];
    size_t number_eid;
    stringstream ss;

    ss << eid;
    ss >> number_eid;
    snprintf(sql, sizeof(sql), "SELECT * FROM exam WHERE exam_id = %lu", number_eid);

    //Exec the SQL query
    PGresult *res;
    res = PQexec(conn, sql);

    //check if the eid is exist
    if( PQntuples(res) == 0)
    {
        err = PC_NOTFOUND;
        PQclear(res);
        return;
    }

    //Set the accomplishment
    snprintf(sql, sizeof(sql), "SELECT  exam_id IN ( "
                                "SELECT exam_id FROM paper WHERE exam_id NOT IN ( "
                                "SELECT exam_id FROM paper WHERE status = false "
                                ") "
                                ") AS CHECK FROM exam WHERE exam_id = %lu;",
                            number_eid);

    res = PQexec(conn, sql);
    if ((PQresultStatus(res) != PGRES_TUPLES_OK))
    {
        err = PC_DBERROR;
        PQclear(res);
        return;
    }

    if (*PQgetvalue(res, 0, 0) == 't')
    {
        accomplishment = "true";
    }

    //Set the management
    if (management == "online" && accomplishment == "true")
    {
        management = "true";
    }
    else
    {
        management = "false";
    }

    //Set the status
    snprintf(sql, sizeof(sql), "SELECT status FROM exam WHERE exam_id = %lu", number_eid);
    res = PQexec(conn, sql);
    string status_before =  PQgetvalue(res, 0, 0);

    if ((status_before == "closed" && status == "finished")||((status_before == "started" && status == "closed"))
        ||(status_before == "finished" && status == "closed"))
        {
            err = PC_CONDITIONERROR;
            PQclear(res);
            return;
        }

    //update the course, exam, start_time, end_time
    snprintf(sql, sizeof(sql),
             "UPDATE exam SET course = '%s', exam_name = '%s', start_time = %lu, end_time = %lu, management = '%s', status = '%s' WHERE exam_id = %lu",
             course.c_str(), exam_name.c_str(), convert_time(start_time.c_str()), convert_time(end_time.c_str()), management.c_str(), status.c_str(),
              number_eid);
    res = PQexec(conn, sql);
    if ((PQresultStatus(res) != PGRES_COMMAND_OK))
    {
        PQclear(res);
        err = PC_SYSTEMERROR;
        return;
    }


    PQclear(res);
    err = PC_SUCCESSFUL;

    return;
}
