#ifndef __QUESTION_H_
#define __QUESTION_H_

#include "common.h"
#include "db.h"
//#include "xml.h"


#include "getGIDByUID.h"
#include "getUIDByCookie.h"

void 
addChoiceToQuestion(const uid_t &userID, const qid_t &questionID,
        const cid_t &choiceID, int &err, PGconn *dbconn);

std::string 
createQuestion(const uid_t &userID, int &err, PGconn *dbconn);

void 
deleteChoiceFromQuestion(const uid_t &userID, const qid_t &questionID,
        const cid_t &choiceID, int &err, PGconn *dbconn);

void 
deleteQuestion(const uid_t &userID, const qid_t &questionID,
        int &err, PGconn *dbconn);

void 
setKeyToQuestion(const uid_t &userID, const qid_t &questionID, 
        const cid_t &choiceID, int &err, PGconn *dbconn);

void 
setQuestionDescription(const uid_t &userID, const qid_t &questionID,
        const std::string description, int &err, PGconn *dbconn);



#endif //__QUESTION_H_
