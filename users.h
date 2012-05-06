#ifndef __INCLUDE_USERS_
#define __INCLUDE_USERS_

#include "common.h"
#include "db.h"
#include "xml.h"

#include <string>

using namespace std;
void changePWD(const uid_t & ope, const string & oldpwd, const string & newpwd,int & err,PGconn * dbconn);

void PwdReset(const uid_t &ope,  const uid_t &resetID,int &err,PGconn *dbconn);

void setUserInfo (const uid_t &ope, const uid_t &usr, const string &info,int &err,PGconn *dbconn);

uid_t userAdd (const string& cookie, const string& text, int &err, PGconn *dbconn);

string userDel (uid_t op, uid_t delusr, int &err, PGconn *dbconn);

void getUserInfo ( const uid_t &user_id, string & info,int &err,PGconn *dbconn);
#endif
