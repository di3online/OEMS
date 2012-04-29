#include "common.h"

const char *
sys_error(const int &err)
{
    const char *res = NULL;
    switch (err)
    {
        case SUCCESSFUL:
            res = "200 OK";
            break;
        case NOPERMISSION:
            res = "400 NO PERMISSION";
            break;
        case INPUTFORMATERROR:
            res = "401 INPUT FORMAT ERROR";
            break;
        case MISTATCH:
            res = "403 MISMATCH";
            break;
        case NOTFOUND:
            res = "404 NOT FOUND";
            break;
        case DBERROR:
            res = "500 DB ERROR";
            break;
        case SYSTEMERROR:
            res = "501 SYSTEM ERROR";
            break;
        default:
            res = "505 UNKNOWN ERROR";
            break;
    }
    return res;

}
