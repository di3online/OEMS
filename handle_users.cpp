#include "handle_users.h"

#include <cstring>

using namespace std;
string handle_USRINF(const string &rawtext)
{
    string cr="\r\n";
    string ret,info,user_cookie;
    uid_t user_id,ope_id;
    DB db;
    PGconn *conn = db.getConn();

    int err = PC_SUCCESSFUL;
    string feedback = "nothing";
    size_t start = 0;
    size_t end = rawtext.find(' ');

    string title = rawtext.substr(start, end - start);

    if(title!= "USRINF")
    {
        err = PC_INPUTFORMATERROR;
        ret = sys_error(err)+cr+cr;
        return ret;
    }

    start = end + 1;
    end = rawtext.find("\r\n");

    user_id= rawtext.substr(start, end - start);

    start = end + 2;
    end = rawtext.find(' ', start);

    title = rawtext.substr(start, end - start);


    if(title!= "Cookie:")
    {
        err = PC_INPUTFORMATERROR;
        ret = sys_error(err)+cr+cr;
        return ret;
    }


    start = end + 1;
    end = rawtext.find("\r\n", start);
    user_cookie = rawtext.substr(start, end - start);



    ope_id = getUIDByCookie(user_cookie,err,conn);
    if(err!= PC_SUCCESSFUL)
    {
        ret = sys_error(err)+cr+cr;
        return ret;
    }
    int temp = getGIDByUID(ope_id,err,conn);
    if(err!=PC_SUCCESSFUL)
    {
        ret = sys_error(err)+cr+cr;
        return ret;
    }
    //Allow teacher to get other users' info 
    //Updated by: Lai
    if (!( temp==GID_ADMIN || ope_id == user_id || temp == GID_TEACHER))
    {
        err =PC_NOPERMISSION;
        ret = sys_error(err)+cr+cr;
        return ret;
    }
    getUserInfo(user_id,feedback,err,conn);
    if(err!=PC_SUCCESSFUL)
    {
        ret=sys_error(err)+cr+cr;
        return ret;
    }
    ret=sys_error(err)+cr+feedback;
    return ret;


}

string handle_MUSRINF(const string &rawtext)
{
    string cr="\r\n";
    string ret,info,user_cookie;
    uid_t user_id,ope_id;
    size_t start = 0;
    size_t end = rawtext.find(' ');
    int err = PC_SUCCESSFUL;
    DB db;
    PGconn *conn = db.getConn();

    string title = rawtext.substr(start, end - start);
    if(title!="MUSRINF")
    {
        err = PC_INPUTFORMATERROR;
        ret = sys_error(err)+cr+cr;
        return  ret;
    }
    start = end + 1;
    end = rawtext.find("\r\n", start);


    user_id = rawtext.substr(start, end - start);

    start = end + 2;
    end = rawtext.find(' ', start);
    title = rawtext.substr(start, end - start);
    if(title != "Cookie:")
    {
        err = PC_INPUTFORMATERROR;
        ret = sys_error(err)+cr+cr;
        return ret;
    }

    start = end + 1;
    end = rawtext.find("\r\n\r\n", start);


    user_cookie = rawtext.substr(start, end - start);

    start = end + 4;
    end = sizeof(rawtext);

    info = rawtext.substr(start,end - start);

    ope_id = getUIDByCookie(user_cookie,err,conn);
    if(err!= PC_SUCCESSFUL)
    {
        ret = sys_error(err)+cr+cr;
        return ret;
    }
    int temp = getGIDByUID(ope_id,err,conn);
    if(err!=PC_SUCCESSFUL)
    {
        ret = sys_error(err)+cr+cr;
        return ret;
    }
    if (!(temp==GID_ADMIN||ope_id==user_id))
    {
        err =PC_NOPERMISSION;
        ret = sys_error(err)+cr+cr;
        return ret;
    }


    setUserInfo(ope_id,user_id,info,err,conn);
    ret = sys_error(err)+cr+cr;
    return ret;

}

string handle_PWDCHG(const string &rawtext)
{
    string ret,newpwd,oldpwd;
    string cr = "\r\n";
    size_t start = 0;
    size_t end = rawtext.find(' ');
    string title = rawtext.substr(start, end - start);
    int err = PC_SUCCESSFUL;
    DB db;
    PGconn *conn = db.getConn();
    if(title!="PWDCHG")
    {
        err = PC_INPUTFORMATERROR;
        ret = sys_error(err)+cr+cr;
        return  ret;
    }

    start = end +1;
    end = rawtext.find("\r\n");
    string user_id = rawtext.substr(start, end - start);

    start = end + 2;
    end = rawtext.find(": ");
    title = rawtext.substr(start , end - start);

    if(title != "Cookie")
    {
        err = PC_INPUTFORMATERROR;
        ret = sys_error(err)+cr+cr;
        return ret;
    }

    start = end + 2;
    end = rawtext.find("\r\n\r\n", start);

    string  user_cookie = rawtext.substr(start, end - start);

    start = end + 4;
    end = sizeof(rawtext);

    string  xml_info = rawtext.substr(start,end - start);


    string cur_Id = getUIDByCookie(user_cookie,err,conn);
    if(err!=PC_SUCCESSFUL)
    {
        ret= sys_error(err)+cr+cr;
        return ret;
    }
    if(cur_Id!=user_id)
    {
        int cur_Gid = getGIDByUID(cur_Id,err,conn);
        if(cur_Gid != GID_ADMIN)
        {
            if(err!=PC_SUCCESSFUL)
            {
                ret= sys_error(err)+cr+cr;
                return ret;
            }

        err = PC_NOPERMISSION;
        ret = sys_error(err)+cr+cr;
        return ret;
        }
    }


    xmlDocPtr doc;
    const char * buf = xml_info.c_str();

    doc = xmlParseMemory(buf,strlen(buf)+1);
    /*Get the oldpwd and newpwd's values from the xml_info*/
    xmlNodePtr curNode; /*Keep node of the xml tree*/
    xmlChar *szKey;

    /*Get the root Node from tmpdoc*/
    curNode = xmlDocGetRootElement(doc);
    /*check the content of the document*/
    /*Check the type of the root element*/
    if(xmlStrcmp(curNode->name,BAD_CAST"PWDCHG"))
    {
        err = PC_INPUTFORMATERROR;
        ret=sys_error(err)+cr+cr;
        xmlFreeDoc(doc);
        return  ret;
    }
    curNode = curNode->xmlChildrenNode;


    while (curNode != NULL)
    {
        /*compare element nodes,show the content*/
        if( !(xmlStrcmp(curNode->name,(const xmlChar *)"old")))
        {
            szKey = xmlNodeGetContent(curNode);
            //cout<<szKey;
            oldpwd.assign((const char*)szKey);
            xmlFree(szKey);
        }
        else if( !(xmlStrcmp(curNode->name,(const xmlChar *)"new")))
        {
            szKey = xmlNodeGetContent(curNode);
            newpwd.assign((const char*)szKey);
            xmlFree(szKey);
        }
        /*traverse*/
        curNode = curNode->next;
    }
    xmlFreeDoc(doc);

    changePWD(user_id,oldpwd,newpwd,err,conn);
    ret = sys_error(err)+cr+cr;
    return ret;

}

string handle_PWDRST(const string &rawtext)
{
    uid_t user_id;
    uid_t ope_id;
    string user_cookie;
    string cr = "\r\n";
    string ret;
    DB db;
    PGconn *conn = db.getConn();

    int err = PC_SUCCESSFUL;
    size_t start = 0;
    size_t end = rawtext.find(' ');

    string title = rawtext.substr(start, end - start);

    if(title!= "PWDRST")
    {
        err = PC_INPUTFORMATERROR;
        ret = sys_error(err)+cr+cr;
        return ret;
    }

    start = end + 1;
    end = rawtext.find("\r\n");

    user_id= rawtext.substr(start, end - start);

    start = end + 2;
    end = rawtext.find(' ', start);

    title = rawtext.substr(start, end - start);


    if(title!= "Cookie:")
    {
        err = PC_INPUTFORMATERROR;
        ret = sys_error(err)+cr+cr;
        return ret;
    }


    start = end + 1;
    end = rawtext.find("\r\n", start);
    user_cookie = rawtext.substr(start, end - start);



    ope_id = getUIDByCookie(user_cookie,err,conn);
    if(err!=PC_SUCCESSFUL)
    {
        ret = sys_error(err)+cr+cr;
        return ret;
    }
    if(getGIDByUID(ope_id,err,conn)!=GID_ADMIN)
    {
        if(err!=PC_SUCCESSFUL)
       {
            ret = sys_error(err)+cr+cr;
            return ret;
       }
        err = PC_INPUTFORMATERROR;
        ret = sys_error(err)+cr+cr;
        return ret;
    }


    PwdReset(ope_id,user_id,err,conn);
    ret = sys_error(err)+cr+cr;
    return ret;


}

/**
 * @brief handle_userAdd
 *
 * @param file
 *          The file which contain the new userID, password, and the operator's cookie
 * @param op
 *          The operator's user ID will return
 * @param staffID
 *          The new user ID will return
 * @param password
 *          The new password will return
 */

string handle_USRADD(const string& rawtext)
{
    uid_t op;
    string cookie;
    string info;

    size_t start = 0;
    size_t end = rawtext.find_first_of("\r\n", start);

    start = end + 2;
    end = rawtext.find(' ', start);

    start = end + 1;
    end = rawtext.find_first_of("\r\n", start);

    //Get the new password
    cookie = rawtext.substr(start, end - start);

    //start = end + 2;
    //Updated by: Lai
    start = end + 4;
    end = sizeof(rawtext);

    info = rawtext.substr(start, end - start);
    //info = rawtext.substr(start);

    //get the operator~
    //op = getUIDByCookie(cookie, err, db.getConn());

    //cout<<op<<"  \n"<<info<<"  "<<err<<endl;

    //return userAdd(op, staffID, password);

    int err;
    DB db;

    return userAdd(cookie, info, err, db.getConn());
}

/**
 * @brief handle_userDel
 *
 * @param file
 *          The file which contain the operator's user ID and the deleted user ID
 * @param op
 *          The operator's user ID will return
 * @param delusr
 *          The deleted user ID will return
 */

string handle_USRDEL(const string& rawtext)
{
    uid_t op;
    uid_t delusr;
    string ret;

    size_t start = 0;
    size_t end = rawtext.find(' ');

    start = end + 1;
    end = rawtext.find_first_of("\r\n", start);

    //Get the delete userID
    delusr = rawtext.substr(start, end - start);

    start = end + 2;
    end = rawtext.find(' ', start);

    start = end + 1;
    end = rawtext.find_first_of("\r\n", start);

    //Get the cookie
    string cookie = rawtext.substr(start, end - start);

    //Get the operator by cookie

    int err = PC_UNKNOWNERROR;
    DB db;

    op = getUIDByCookie(cookie, err, db.getConn());

    if(err != PC_SUCCESSFUL)
    {
        ret = sys_error(err);
        ret = ret + "\r\n" + "\r\n";
        return ret;
    }

    //cout<<err<endl;

    //cout<<"The delete userID id: "<<delusr<<"\nThe cookie is: "<<cookie<<"\nThe op is: "<<op<<endl;

    return userDel (op, delusr, err, db.getConn());

}

