#ifndef __INCLUDE_CHOICE_
#define __INCLUDE_CHOICE_

#include <string>
#include <sstream>

#include "getUIDByCookie.h"
#include "getGIDByUID.h"
#include "common.h"
#include "db.h"

void 
setChoiceDescription(const uid_t &uid, const cid_t &cid, string description, int &err, PGconn *dbconn);

cid_t 
createChoice(const uid_t &uid, int &err, PGconn *dbconn);
#endif
