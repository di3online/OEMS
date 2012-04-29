#ifndef __INCLUDE_COMMON_
#define __INCLUDE_COMMON_

#include <string>

#define uid_t std::string 
#define pid_t std::string

typedef std::string eid_t, qid_t, cid_t;

//eid_t, pid_t, qid_t, cid_t;

enum ProtocolCode{
    SUCCESSFUL = 0,
    NOPERMISSION,
    INPUTFORMATERROR,
    NOTFOUND,
    MISTATCH,
    DBERROR,
    SYSTEMERROR,
    UNKNOWNERROR
};

const char *
sys_error(const int &err);
#endif


