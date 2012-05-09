
#ifndef __INCLUDE_SCORE_
#define __INCLUDE_SCORE_

//int
//getScore(const uid_t &uid, const eid_t &eid, int &err, PGconn *dbconn);

int 
getScore(const uid_t &uid, const pid_t &pid, int &err, PGconn *dbconn);

#endif
