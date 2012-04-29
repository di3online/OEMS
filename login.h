#ifndef __INCLUDE_LOGIN_
#define __INCLUDE_LOGIN_

#include <string>

#include "db.h"
#include "common.h"

#define MAXLEN_USERID 20
#define MAXLEN_PASSWORD 20
#define MAXLEN_COOKIE 100

std::string 
handle_login(const std::string &rawtext);

std::string 
login(const uid_t &userID, const std::string &password, 
        int &err, PGconn *dbconn);

#endif //__INCLUDE_LOGIN_
