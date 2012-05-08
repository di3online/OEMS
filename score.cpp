#include "common.h"
#include <cstdlib>
#include "score.h"

#include <iostream>

using namespace std;


/*
int
getScore(const uid_t &uid, const eid_t &eid, int &err, PGconn *dbconn)
{
    char query[MAXBUF];
    snprintf(query, sizeof(query), 
            "UPDATE student_exam SET final_score = (  "
                "SELECT (SheetA.AC * 100 / SheetB.ALL ) AS Score  "
                "FROM (  "
                    "SELECT COUNT(*) AS AC FROM answer_sheet, question, student_exam  "
                    "WHERE answer_sheet.user_id = '%s'  "
                    "AND student_exam.user_id = answer_sheet.user_id  "
                    "AND question.question_id = answer_sheet.question_id  "
                    "AND question.paper_id = student_exam.paper_id  "
                    "AND answer_sheet.answer = question.key  "
                    "AND question.paper_id IN (  "
                        "SELECT paper_id FROM paper  "
                        "WHERE exam_id = %s  "
                    ") "
                ") AS SheetA,  "
                "(  "
                    "SELECT COUNT(*) AS ALL  "
                    "FROM question, student_exam  "
                    "WHERE student_exam.user_id = '%s'  "
                    "AND question.paper_id = student_exam.paper_id  "
                    "AND question.paper_id IN (  "
                        "SELECT paper_id FROM paper  "
                        "WHERE exam_id = %s "
                    ") "
                ") AS SheetB  "
            ")  WHERE user_id = '%s' AND paper_id IN (  "
                "SELECT paper_id FROM paper  "
                "WHERE exam_id = %s "
                ") RETURNING final_score , paper_id ",
        uid.c_str(), eid.c_str(), 
        uid.c_str(), eid.c_str(), 
        uid.c_str(), eid.c_str());

    PGresult *sqlres = PQexec(dbconn, query);


    if (PGRES_TUPLES_OK != PQresultStatus(sqlres))
    {
        err = PC_DBERROR;
        PQclear(sqlres);
        return -1;
    }

    if (PQntuples(sqlres) == 0 || PQgetisnull(sqlres, 0, 0))
    {
        err = PC_DBERROR;
        PQclear(sqlres);
        return -1;
    }

    int score = strtol(PQgetvalue(sqlres, 0, 0), NULL, 10);

    PQclear(sqlres);

    err = PC_SUCCESSFUL;

    return score;
}*/

int getScore(const uid_t &uid, const pid_t &pid, int &err, PGconn *dbconn)
{

    char query[MAXBUF];
    snprintf(query, sizeof(query), 
            "UPDATE student_exam SET final_score = ( "
                "SELECT (SheetA.AC * 100 / SheetB.ALL ) AS Score  "
                "FROM (  "
                    "SELECT COUNT(*) AS AC FROM answer_sheet, question, student_exam  "
                    "WHERE answer_sheet.user_id = '%s'  "
                    "AND student_exam.user_id = answer_sheet.user_id  "
                    "AND question.question_id = answer_sheet.question_id  "
                    "AND question.paper_id = student_exam.paper_id  "
                    "AND answer_sheet.answer = question.key  "
                    "AND question.paper_id = %s "
                ") AS SheetA,  "
                "(  "
                    "SELECT COUNT(*) AS ALL  "
                    "FROM question, student_exam  "
                    "WHERE student_exam.user_id = '%s'  "
                    "AND question.paper_id = student_exam.paper_id  "
                    "AND question.paper_id = %s "
                ") AS SheetB  "
            ")  WHERE user_id = '%s' AND paper_id  = %s "
            "RETURNING final_score , paper_id ",
        uid.c_str(), pid.c_str(), 
        uid.c_str(), pid.c_str(), 
        uid.c_str(), pid.c_str());

    PGresult *sqlres = PQexec(dbconn, query);


    if (PGRES_TUPLES_OK != PQresultStatus(sqlres))
    {
        err = PC_DBERROR;
        cerr << PQerrorMessage(dbconn) << endl;
        cerr.flush();

        PQclear(sqlres);
        return -1;
    }

    if (PQntuples(sqlres) == 0 || PQgetisnull(sqlres, 0, 0))
    {
        err = PC_DBERROR;
        PQclear(sqlres);
        return -1;
    }

    int score = strtol(PQgetvalue(sqlres, 0, 0), NULL, 10);

    PQclear(sqlres);

    err = PC_SUCCESSFUL;

    return score;

}
