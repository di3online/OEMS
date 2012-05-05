#ifndef __INCLUDED_HANDERS_
#define __INCLUDED_HANDERS_

#include <string>

#include "login.h"


std::string handle_LSTET(const std::string &rawtext);
std::string handle_EINF(const std::string &rawtext);
std::string handle_UPANS(const std::string &rawtext);

#endif
