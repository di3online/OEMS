
#ifndef __INCLUDE_PAPER_
#define __INCLUDE_PAPER_

#include <string>
#include "common.h"
#include "db.h"

#include "getUIDByCookie.h"
#include "getGIDByUID.h"

void 
addQuestionToPaper(const uid_t &uid, const qid_t &qid, const pid_t &pid, int &err, PGconn *dbconn);

pid_t 
createPaper(const uid_t &uid, int &err, PGconn *dbconn);

void
deletePaper(const uid_t &uid, const pid_t &pid, int &err, PGconn *dbconn);

#endif
