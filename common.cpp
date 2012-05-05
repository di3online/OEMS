
/**
 * @file common.cpp
 * @brief 
 * @author LaiSHZH
 * @version 1.0
 * @date 2012-05-01
 */

#include "common.h"

#include <cstdio>
#include <cstdlib>


/**
 * @brief 
 *          convert the gid to the name of its group
 *
 * @param gid 
 *          the group id
 *
 * @return  
 *          the name of group id
 */
const char *
echo_gid(const int &gid)
{
    const char *res = NULL;
    switch (gid)
    {
        case GID_ADMIN:
            res = "admin";
            break;
        case GID_TEACHER:
            res = "teacher";
            break;
        case GID_STUDENT:
            res = "student";
            break;
        case GID_UNKNOWN:
            //pass
        default:
            res = "unkown";
            break;
    }
    return res;
}


/**
 * @brief 
 *          convert the error code into the text description
 *
 * @param err
 *          error code
 *
 * @return 
 *          the text description
 */
const char *
sys_error(const int &err)
{
    const char *res = NULL;
    switch (err)
    {
        case PC_SUCCESSFUL:
            res = "200 OK";
            break;
        case PC_NOPERMISSION:
            res = "400 NO PERMISSION";
            break;
        case PC_INPUTFORMATERROR:
            res = "401 INPUT FORMAT ERROR";
            break;
        case PC_MISTATCH:
            res = "403 MISMATCH";
            break;
        case PC_NOTFOUND:
            res = "404 NOT FOUND";
            break;
        case PC_CONDITIONERROR:
            res = "405 CONDITION ERROR";
            break;
        case PC_DBERROR:
            res = "500 DB ERROR";
            break;
        case PC_SYSTEMERROR:
            res = "501 SYSTEM ERROR";
            break;
        case PC_UNKNOWNERROR:
            //pass
        default:
            res = "505 UNKNOWN ERROR";
            break;
    }
    return res;

}

/**
 * @brief 
 *      convert the time string to time_t
 *
 * @param str
 *      time string whose format is like 
 *      2012-05-03 22:33:13
 */
time_t
convert_time(const char *str)
{
    struct tm time;

    sscanf(str, "%d-%d-%d %d:%d:%d", 
            &time.tm_year, 
            &time.tm_mon,
            &time.tm_mday,
            &time.tm_hour,
            &time.tm_min,
            &time.tm_sec);
    time.tm_year -= 1900;
    time.tm_mon -= 1;

    return mktime(&time);

}

void 
convert_time(char *to, size_t size, const char *from)
{
    time_t timeval = strtol(from, NULL, 10);
    struct tm time;

    localtime_r(&timeval, &time);

    strftime(to, size, "%F %T", &time);

    return ;
}

