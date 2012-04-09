#ifndef __INCLUDE_LOGIN_
#define __INCLUDE_LOGIN_

#include <string>

#define uid_t std::string


#define MAXLEN_USERID 20
#define MAXLEN_PASSWORD 20

void handle_login(const std::string &rawtext, 
        uid_t &userID, std::string &password);
std::string login(const uid_t &userID, const std::string &password);

#endif //__INCLUDE_LOGIN_
