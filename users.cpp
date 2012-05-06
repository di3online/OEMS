#include "users.h"

#include <cstring>

using namespace std;

void changePWD(const uid_t & ope, const string & oldpwd, const string & newpwd,int &err,PGconn *dbconn)
{
    //string ret;

    PGresult *res;

    if (PQstatus(dbconn) != CONNECTION_OK)
    {
       //cout<<connect fail;
       err= PC_DBERROR;
       return ;
    }


    char cuserID[2 * ope.size() + 1];

    PQescapeString(cuserID, ope.c_str(), ope.size());

    string t = cuserID;

    string sql = "SELECT password FROM users WHERE user_id ='"+t+"'";
    res = PQexec(dbconn, sql.c_str());


    if (PQresultStatus(res) != PGRES_TUPLES_OK){
        /*ret = PQresStatus(PQresultStatus(res));
        cout<<ret;*/
        err = PC_DBERROR;
        return ;
    }

    string db_oldpwd = PQgetvalue(res, 0, 0);


    PQclear(res);

    if(db_oldpwd!=oldpwd)
    {
        err = PC_NOPERMISSION;
        return ;
    }


    sql = "UPDATE users SET password = '"+newpwd+"' WHERE user_id ='"+t+"'";
    res = PQexec(dbconn, sql.c_str());

    if (PQresultStatus(res) != PGRES_COMMAND_OK){
       /* ret = PQresStatus(PQresultStatus(res));
       cout<<ret;*/
        err = PC_DBERROR;
        return ;
    }

    PQclear(res);



    return;

}


void PwdReset(const uid_t &ope,  const uid_t &resetID,int & err,PGconn *dbconn)
{
    //string ret;

    if(getGIDByUID(ope,err,dbconn)!=GID_ADMIN)
    {
        if(err!=PC_SUCCESSFUL)
        {
            return;
        }
        err = PC_NOPERMISSION;
        return;
    }
    if(getGIDByUID(ope,err,dbconn)==GID_UNKNOWN)
    {
        return;
    }



    PGresult *res;


    if (PQstatus(dbconn) != CONNECTION_OK)
    {
       err = PC_DBERROR;
       return ;
    }


    char cuser[2 * resetID.size() + 1];

    PQescapeString(cuser, resetID.c_str(), resetID.size());

    string t = cuser;
    string blank = "";
    string sql ="UPDATE users SET password = DEFAULT WHERE user_id ='"+t+"'";

    res = PQexec(dbconn, sql.c_str());

    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        /*ret = PQresStatus(PQresultStatus(res));
        cout<<ret;*/
        err = PC_DBERROR;
        return ;
    }

    PQclear(res);


    return ;

}


void setUserInfo (const uid_t &ope, const uid_t &usr, const string &info,int &err,PGconn* dbconn)
{

    //string ret;


    int ope_gid = getGIDByUID(ope,err,dbconn);
    if(err!=PC_SUCCESSFUL)
    {
        return;
    }



    if(getGIDByUID(usr,err,dbconn)==GID_UNKNOWN)
    {
        return;
    }
    if(err!=PC_SUCCESSFUL)
    {
        return;
    }
    if(!(ope==usr||ope_gid==GID_ADMIN))
    {
        err=PC_NOPERMISSION;
        return;
    }


    PGresult *res;


    if (PQstatus(dbconn) != CONNECTION_OK)
    {
       err = PC_DBERROR;
       return ;
    }



    char cuserID[2 * usr.size() + 1];

    PQescapeString(cuserID, usr.c_str(), usr.size());

    string t = cuserID;
    string sql;



    xmlDocPtr doc;
    const char * buf = info.c_str();
    doc = xmlParseMemory(buf,strlen(buf)+1);
    /*Get the uder's information from the info*/
    xmlNodePtr curNode; /*Keep node of the xml tree*/
    xmlChar *szKey;

    /*Get the root Node from tmpdoc*/
    curNode = xmlDocGetRootElement(doc);
    /*check the content of the document*/
    /*Check the type of the root element*/
    if(xmlStrcmp(curNode->name,BAD_CAST"MUSRINF"))
    {
        err = PC_INPUTFORMATERROR;
        xmlFreeDoc(doc);
        return ;
    }
    curNode = curNode->xmlChildrenNode;
    //xmlNodePtr propNpdePtr =curNode;
    sql = "BEGIN";
    res = PQexec(dbconn, sql.c_str());
    PQclear(res);



        while (curNode != NULL)
     {

        /*compare element nodes,show the content*/
        if( !(xmlStrcmp(curNode->name,(const xmlChar *)"phone")))
        {
            szKey = xmlNodeGetContent(curNode);
            string temp((const char*)szKey);
            sql = "UPDATE users SET telephone = '"+temp+"' WHERE user_id ='"+t+"'";
            res = PQexec(dbconn, sql.c_str());

            if (PQresultStatus(res) != PGRES_COMMAND_OK){
               /* ret = PQresStatus(PQresultStatus(res));
               cout<<ret;*/
                err = PC_DBERROR;
                sql = " ROLLBACK";
                res = PQexec(dbconn, sql.c_str());
                PQclear(res);
                return ;
            }

            PQclear(res);

            xmlFree(szKey);
        }
        else if( !(xmlStrcmp(curNode->name,(const xmlChar *)"email")))
        {
            szKey = xmlNodeGetContent(curNode);
            string temp((const char*)szKey);
            sql = "UPDATE users SET email = '"+temp+"' WHERE user_id ='"+t+"'";
            res = PQexec(dbconn, sql.c_str());

            if (PQresultStatus(res) != PGRES_COMMAND_OK){
                /*ret = PQresStatus(PQresultStatus(res));
                cout<<ret;*/
                err = PC_DBERROR;
                sql = " ROLLBACK";
                res = PQexec(dbconn, sql.c_str());
                PQclear(res);
                return;
            }

            PQclear(res);

            xmlFree(szKey);
        }

        /*traverse*/
        curNode = curNode->next;
    }
    sql = "COMMIT";
    res = PQexec(dbconn, sql.c_str());
    PQclear(res);

    xmlFreeDoc(doc);


    return ;





}




/**
 * @brief userAdd
 *          Add a new user
 * @param op
 *          The operator's user ID
 * @param staffID
 *          New user ID for a student, it's a student ID
 * @param password
 *          md5 encrypted password
 *
 * @return
 *          a string, see protocol document for detail
 */

string userAdd (const string& cookie, const string& info, int &err, PGconn *dbconn)
{
    DB db;
    err = PC_UNKNOWNERROR;
    PGconn *conn = dbconn;
    PGresult* res;

    string ret;

    uid_t op = getUIDByCookie(cookie, err, db.getConn());

    if(err != PC_SUCCESSFUL)
    {
        ret = sys_error(err);
        ret = ret + "\r\n" + "\r\n";
        return ret;
    }

    //Get the operator's group id;
    gid_t i = getGIDByUID(op, err, db.getConn());

    if(err != PC_SUCCESSFUL)
    {
        ret = sys_error(err);
        ret = ret + "\r\n" + "\r\n";
        return ret;
    }

    if(i != GID_ADMIN)
    {//If the operator haven't the right to add a new user
        err = PC_NOPERMISSION;
        ret = sys_error(err);
        ret = ret + "\r\n" + "\r\n";
        return ret;
    }

    xmlDocPtr doc;
    const char * buf = info.c_str();

    doc = xmlParseMemory(buf,strlen(buf)+1);
    /*Get the oldpwd and newpwd's values from the xml_info*/
    xmlNodePtr curNode; /*Keep node of the xml tree*/
    xmlChar *szKey;

    /*Get the root Node from tmpdoc*/
    curNode = xmlDocGetRootElement(doc);
    /*check the content of the document*/
    /*Check the type of the root element*/


    if(xmlStrcmp(curNode->name,BAD_CAST"USRADD"))
    {
        err = PC_INPUTFORMATERROR;
        ret = sys_error(err);
        ret = ret + "\r\n" + "\r\n";
        xmlFreeDoc(doc);
        return  ret;
    }
    curNode = curNode->xmlChildrenNode;

    string userID, groupID, userName, userPhoto;


    while (curNode != NULL)
    {
        /*compare element nodes,show the content*/
        if( !(xmlStrcmp(curNode->name,(const xmlChar *)"uid")))
        {
            szKey = xmlNodeGetContent(curNode);
            //cout<<szKey;
            userID.assign((const char*)szKey);
            xmlFree(szKey);
        }
        else if( !(xmlStrcmp(curNode->name,(const xmlChar *)"gid")))
        {
            szKey = xmlNodeGetContent(curNode);
            groupID.assign((const char*)szKey);
            xmlFree(szKey);
        }
        else if( !(xmlStrcmp(curNode->name,(const xmlChar *)"name")))
        {
            szKey = xmlNodeGetContent(curNode);
            userName.assign((const char*)szKey);
            xmlFree(szKey);
        }
        else if( !(xmlStrcmp(curNode->name,(const xmlChar *)"photo")))
        {
            szKey = xmlNodeGetContent(curNode);
            userPhoto.assign((const char*)szKey);
            xmlFree(szKey);
        }
        /*traverse*/
        curNode = curNode->next;
    }
    xmlFreeDoc(doc);

    //escapte string of userID and password
    /*
    char cusr_id[2 * staffID.size() + 1];
    char cnpasswd[2 * password.size() + 1];

    PQescapeString(cusr_id, staffID.c_str(), staffID.size());
    PQescapeString(cnpasswd, password.c_str(), password.size());

    string user_id = cusr_id;
    string npasswrd = cnpasswd;*/

    char cuserID[2 * userID.size() + 1];
    PQescapeString(cuserID, userID.c_str(), userID.size());
    userID = cuserID;

    //Add by: Lai
    if (groupID == "admin") {
        groupID = "0";
    } else if (groupID == "teacher") {
        groupID = "1";
    } else if (groupID == "student") {
        groupID = "2";
    } else {
        err = PC_INPUTFORMATERROR;
        ret = sys_error(err);
        ret += "\r\n\r\n";
        return ret;
    }

    char cgroupID[2 * groupID.size() + 1];
    PQescapeString(cgroupID, groupID.c_str(), groupID.size());
    groupID = cgroupID;

    char cuserName[2 * userName.size() + 1];
    PQescapeString(cuserName, userName.c_str(), userName.size());
    userName = cuserName;

    char cuserPhoto[2 * userID.size() + 1];
    PQescapeString(cuserPhoto, userPhoto.c_str(), userPhoto.size());
    userPhoto = cuserPhoto;

    string sql = "insert into users (user_id, group_id, name, sex, photo_url) values ('" + userID + "'," + groupID + ",'" + userName + "','Male','" + userPhoto + "')";

    cout<<userID<<" "<<groupID<<" "<<userName<<" "<<userPhoto<<endl;
    cout<<sql<<endl;

    //Exec the SQL query
    res = PQexec(conn, sql.c_str());

    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        //If exection failed
        err = PC_DBERROR;
        ret = sys_error(err);
        ret = ret + "\r\n" + "\r\n";
        PQclear(res);
        return ret;
    }

    //Free the result of SQL
    PQclear(res);

    err = PC_SUCCESSFUL;
    ret = sys_error(err);
    ret = ret + "\r\n" + "\r\n";

    return ret;
}



/**
 * @brief userDel
 *          Delete a user
 * @param op
 *          The operator's user ID
 * @param delusr
 *          The deleted user ID
 * @return
 *          a string, see protocol document for detail
 */

string userDel (uid_t op, uid_t delusr, int &err, PGconn *dbconn)
{
    PGconn *conn = dbconn;
    PGresult* res;
    string ret;

    err = PC_UNKNOWNERROR;

    if (PQstatus(conn) != CONNECTION_OK)
    {
        err = PC_DBERROR;
        ret = sys_error(err);
        ret = ret + "\r\n" + "\r\n";
        return ret;
    }

    //Get the operator's group id;
    gid_t i = getGIDByUID(op, err, conn);

    if(i != GID_ADMIN)
    {
        //If the operator haven't the right to delete a user
        err = PC_NOPERMISSION;
        ret = sys_error(err);
        ret = ret + "\r\n" + "\r\n";
        return ret;
    }

    //escapte string of deleted user ID
    char cDelusr[2 * delusr.size() + 1];

    PQescapeString(cDelusr, delusr.c_str(), delusr.size());

    string t = cDelusr;

    //Exec the SQL query
    string sql = "SELECT * FROM users WHERE user_id ='"+t+"'";
    res = PQexec(conn, sql.c_str());


    if (PQntuples(res) == 0)
    {
        //The deleted user ID not exist
        err = PC_NOTFOUND;
        ret = sys_error(err);
        ret = ret + "\r\n" + "\r\n";
        PQclear(res);
        return ret;
    }

    sql = "DELETE FROM users WHERE user_id = '" + t + "'";

    //Exec the SQL query
    res = PQexec(conn, sql.c_str());

    //Updated By: Lai
    //change PGRES_TUPLES_OK to PGRES_COMMAND_OK
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {//If exection failed
        err = PC_DBERROR;
        ret = sys_error(err);
        ret = ret + "\r\n" + "\r\n";
        PQclear(res);
        return ret;
    }

    //Free the result of SQL
    PQclear(res);

    //exection Ok
    err = PC_SUCCESSFUL;
    ret = sys_error(err);
    ret = ret + "\r\n" + "\r\n";

    return ret;

}



void getUserInfo ( const uid_t &user_id, string & info,int &err,PGconn *dbconn)
{
    string cr = "\r\n";

    string db_name,db_phone,db_email;


    //Init the db connection
    PGresult *res = PQexec(dbconn, "BEGIN");

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        err = PC_DBERROR;
        PQclear(res);
        return ;
    }
    PQclear(res);

    char cuser[2 * user_id.size() + 1];

    PQescapeString(cuser, user_id.c_str(), user_id.size());

    string t = cuser;
    //get user's name from db
    string sql = "SELECT name FROM users WHERE user_id ='"+t+"'";
    res = PQexec(dbconn, sql.c_str());


    if (PQresultStatus(res) != PGRES_TUPLES_OK){
        /*ret = PQresStatus(PQresultStatus(res));
        cout<<ret;*/
        err = PC_DBERROR;
        PQclear(res);
        PQexec(dbconn, "ROLLBACK");

        return ;
    }

    db_name = PQgetvalue(res, 0, 0);


    PQclear(res);
    //get the user's email from db
    sql = "SELECT email FROM users WHERE user_id ='"+t+"'";
    res = PQexec(dbconn, sql.c_str());


    if (PQresultStatus(res) != PGRES_TUPLES_OK){
        /*ret = PQresStatus(PQresultStatus(res));
        cout<<ret;*/
        err = PC_DBERROR;
        PQclear(res);
        PQexec(dbconn, "ROLLBACK");

        return ;
    }

    db_email = PQgetvalue(res, 0, 0);


    PQclear(res);
    //get user's phone from db
    sql = "SELECT telephone FROM users WHERE user_id ='"+t+"'";
    res = PQexec(dbconn, sql.c_str());


    if (PQresultStatus(res) != PGRES_TUPLES_OK){
        /*ret = PQresStatus(PQresultStatus(res));
        cout<<ret;*/
        err = PC_DBERROR;
        PQclear(res);
        PQexec(dbconn, "ROLLBACK");

        return ;
    }

    db_phone = PQgetvalue(res, 0, 0);


    PQclear(res);

    res = PQexec(dbconn, "COMMIT");
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        err=PC_DBERROR;
        PQexec(dbconn, "ROLLBACK");
        return ;
    }
    PQclear(res);



    //Generate the XML body
    xmlDocPtr doc = NULL;
    xmlNodePtr root_node = NULL;

    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "USRINF");

    xmlDocSetRootElement(doc, root_node);
    xmlNewChild(root_node, NULL, BAD_CAST "uid",
                        BAD_CAST user_id.c_str());

    xmlNewChild(root_node, NULL, BAD_CAST "name",
                        BAD_CAST db_name.c_str());

    xmlNewChild(root_node, NULL, BAD_CAST "email",
                        BAD_CAST db_email.c_str());

    xmlNewChild(root_node, NULL, BAD_CAST "phone",
                        BAD_CAST db_phone.c_str());

    xmlChar *xmlbuffer;
    int buffersize;

    xmlDocDumpFormatMemory(doc, &xmlbuffer, &buffersize, 1);

    info =cr+(char *)xmlbuffer;


    xmlFree(xmlbuffer);
    xmlFreeDoc(doc);



}
