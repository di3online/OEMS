#ifndef __INCLUDE_EXAM_
#define __INCLUDE_EXAM_

#include <string>

#include "db.h"
#include "common.h"
#include "getGIDByUID.h"
#include "getUIDByCookie.h"

#define MAXLEN_COOKIE 100
#define MAXLEN_EXAMID 20
#define MAXLEN_PAPERID 20

void 
addPaperToExam(const uid_t &uid, const pid_t &pid, const eid_t &eid,
        int &err, PGconn *dbconn);

eid_t 
createExam(const uid_t &uid, int &err, PGconn *dbconn);

void
deleteExam(const uid_t &uid, const eid_t &eid, int &err, PGconn *dbconn);

void setExamInfo(const uid_t &uid, const eid_t &eid,
                std::string exam_name, std::string start_time, std::string end_time,
                std::string management,std::string status, std::string course,
                int &err, PGconn *dbconn);


#endif
