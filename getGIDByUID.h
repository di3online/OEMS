/**
 * @file getCIDByUID.h
 * @brief
 * @author MoShuqi
 * @version 1.0
 * @date 2012-04-12
 */

#ifndef GETGIDBYUID_H_INCLUDED
#define GETGIDBYUID_H_INCLUDED

#include <iostream>
#include <string>
#include <sstream>
#include <postgresql/libpq-fe.h>
#include "db.h"
#include "common.h"

#define uid_t std::string
#define MAXLEN_USERID 20

using std::string;
using std::cout;
using std::endl;

string handle_getGIDByUID(std::string &rawtext);

gid_t getGIDByUID(const uid_t &userID, int &err, PGconn *dbconn);

#endif // GETGIDBYUID_H_INCLUDED
