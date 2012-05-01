/**
 * @file getUIDByCookie.h
 * @brief
 * @author MoShuqi
 * @version 1.0
 * @date 2012-04-12
 */

#ifndef GETUIDBYCOOKIE_H_INCLUDED
#define GETUIDBYCOOKIE_H_INCLUDED

#include <iostream>
#include <string>
#include <postgresql/libpq-fe.h>
#include "db.h"
#include "common.h"

#define uid_t std::string
#define MAXLEN_COOKIESIZE 100

using std::string;
using std::cout;
using std::endl;

//const size_t MAXLEN_COOKIE = 100;

//void handle_getUIDByCookie();  No need to build this function.

/**
 * @brief getUIDByCookie
 *
 * @param cookie
 *          Get a group id by the cookie
 * @return
 *          The group id of the cookie
 */

uid_t getUIDByCookie(const string &cookie, int &err, PGconn *dbconn);


#endif // GETUIDBYCOOKIE_H_INCLUDED
