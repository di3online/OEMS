#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <postgresql/libpq-fe.h>

#include "login.h"

using namespace std;

int main(int argc, char *argv[])
{
    //file input
    ifstream fin;
    fin.open("login.in", std::ios_base::in);

    stringstream ss;
    ss << fin.rdbuf();
    ss.flush();
    string rawtext = ss.str();
    
    fin.close();

    //cout << rawtext;
    //cout.flush();

    string result = handle_login(rawtext);
    
    cout << result;

    cout.flush();

    return 0;
}
