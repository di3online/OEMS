#ifndef __INCLUDE_HANDLE_USERS_
#define __INCLUDE_HANDLE_USERS_

#include <string>
#include "handlers.h"
#include "users.h"

std::string handle_PWDCHG(const std::string &rawtext);

std::string handle_USRDEL(const std::string &rawtext);

std::string handle_USRADD(const std::string &rawtext);

std::string handle_PWDRST(const std::string &rawtext);

std::string handle_USRINF(const std::string &rawtext);

std::string handle_MUSRINF(const std::string &rawtext);

#endif
