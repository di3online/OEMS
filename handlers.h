#ifndef __INCLUDED_HANDERS_
#define __INCLUDED_HANDERS_

#include <string>

#include "login.h"
#include "getUIDByCookie.h"
#include "getGIDByUID.h"
#include "db.h"
#include "common.h"
#include "xml.h"

#include "exam.h"
#include "paper.h"
#include "question.h"
#include "choice.h"


#include "handle_users.h"

std::string handle_LSTE(const std::string &rawtext);

std::string handle_LSTES(const std::string &rawtext);

std::string handle_LSTET(const std::string &rawtext);

std::string handle_EINF(const std::string &rawtext);
std::string handle_UPANS(const std::string &rawtext);

std::string handle_ADDE(const std::string &rawtext);
std::string handle_MEINF(const std::string &rawtext);
std::string handle_MPSTA(const std::string &rawtext);

std::string handle_ADDQ(const std::string &rawtext);
std::string handle_MQINF(const std::string &rawtext);
#endif
