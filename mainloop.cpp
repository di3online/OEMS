#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include <string>
#include <iostream>

#include "login.h"

#define LISTENQ 1024
#define MAXADDR 20
#define MAXBUF 4048
#define MAXSIZELEN 8

using namespace std;

const static int SERV_PORT = 8083;

static string child_recv(int sockfd);
static string child_distribute(string &request);
static void child_send(int sockfd, string &data);

int main()
{
    int ret = 0;

    int listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (listen_fd < 0)
    {
        perror("mainloop: socket failed");
    }
    
    struct sockaddr_in servaddr;

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    ret = bind(listen_fd, (const struct sockaddr *) &servaddr, sizeof(servaddr));

    if (ret < 0)
    {
        perror("mainloop: bind failed");
    }

    ret = listen(listen_fd, LISTENQ);

    if (ret < 0)
    {
        perror("mainloop: listen failed");
    }
    
    struct sockaddr_in cliaddr;
    //socklen_t len;
    while (1)
    {

        //int connfd = accept(listen_fd, (struct sockaddr *) &cliaddr, &len);

        int connfd = accept(listen_fd, NULL, NULL);

        if (connfd < 0)
        {
            perror("mainloop: accept failed");
        }
        if (fork() == 0)
        {
            
            cerr << "Accept new client" << endl;
            cerr.flush();
            //Close child's listen fd, parent continue to listen it.
            close(listen_fd);

            char addr[MAXADDR];
            inet_ntop(AF_INET, &cliaddr.sin_addr, addr, sizeof(addr));

            string str_body = child_recv(connfd);

            string result = child_distribute(str_body);

            child_send(connfd, result);

            
            exit(0);

        }
        //Close parent's connection fd, child will handle this.
        close(connfd);
        
    }

    return 0;

}

static 
string 
child_recv(int sockfd)
{
    string request;
    static char buffer[MAXBUF];

    int nread = read(sockfd, buffer, MAXSIZELEN);
    buffer[nread] = '\0';

    int size_to_read = atoi(buffer);

    int cur_size = 0;

    while (size_to_read > 0)
    {
        cur_size = read(sockfd, buffer, MAXBUF - 1);
        if (cur_size < 0 && errno == EINTR)
        {
            continue;
        } else if (cur_size <= 0) {
            close(sockfd);
            request = "500 SOCKETREAD ERROR\r\n\r\n";
            cerr << "socket read fail" << endl;
            cerr.flush();
            break;
        }
        
        buffer[cur_size] = '\0';
        request += buffer;
        size_to_read -= cur_size;
    }
    
    return request;
}

static 
string 
child_distribute(string &request)
{
    string result;
    size_t loc = request.find_first_of(" \r\n");
    string command = request.substr(0, loc);
    cout << request << endl;
    cout.flush();

    if (command == "LOGIN"){
        cout << "enter handle" << endl;
        cout.flush();
        result = handle_login(request);
        cout << result << endl;
        cout.flush();
    } else if (command == "500"){
        result = request;
    } else {
        result = "500 DISTRUBUTE COMMAND NOT FOUND\r\n\r\n";
        cout << result << endl;
    }

    return result;
}

static 
void 
child_send(int sockfd, string &data)
{
    char buf_size[MAXSIZELEN];
    snprintf(buf_size, sizeof(buf_size), "%lu", data.size() + 1);
    write(sockfd, buf_size, sizeof(buf_size));


    size_t len_write;
    size_t len_start = 0;
    while ( len_start < data.size() + 1)
    {
        len_write = write(sockfd, 
                data.c_str() + len_start, data.size() + 1);
        if (len_write < 0)
        {
            if (errno == EINTR)
            {
                continue ;
            }
            close(sockfd);
            perror("mainloop: write fail");
            break;
        }
        len_start += len_write;
    }

    close(sockfd);

    return ;
}
