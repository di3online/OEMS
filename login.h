#ifndef __INCLUDE_LOGIN_
#define __INCLUDE_LOGIN_

#include <string>

#define uid_t std::string


#define MAXLEN_USERID 20
#define MAXLEN_PASSWORD 20

std::string handle_login(const std::string &rawtext);
std::string login(const uid_t &userID, const std::string &password);

#endif //__INCLUDE_LOGIN_
