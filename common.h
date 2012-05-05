/**
 * @file common.h
 * @brief 
 * @author LaiSHZH
 * @version 1.0
 * @date 2012-05-01
 */
#ifndef __INCLUDE_COMMON_
#define __INCLUDE_COMMON_


#include <string>

#include "getGIDByUID.h"
#include "getUIDByCookie.h"

//System has its definition of uid_t, pid_t, gid_t
#define uid_t std::string 
#define pid_t std::string
#define gid_t int

typedef std::string eid_t, qid_t, cid_t;

enum GidList{
    GID_ADMIN = 0,
    GID_TEACHER,
    GID_STUDENT,
    GID_UNKNOWN
};

const char *
echo_gid(const int &gid);

//eid_t, pid_t, qid_t, cid_t;

enum ProtocolCodeList{
    PC_SUCCESSFUL = 0,

    //User error list
    PC_NOPERMISSION,
    PC_INPUTFORMATERROR,
    PC_NOTFOUND,
    PC_MISTATCH,
    PC_CONDITIONERROR,

    //System error list
    PC_DBERROR,
    PC_SYSTEMERROR,
    PC_UNKNOWNERROR
};

const char *
sys_error(const int &err);

time_t convert_time(const char *str);
void convert_time(char *to, size_t size, const char *from);

#define MAXBUF 4096
#endif


