/**
 * @file mainloop.cpp
 * @brief 
 * @author Lai
 * @version 2.0
 * @date 2012-05-02
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include <semaphore.h>
#include <sys/mman.h>
#include <pthread.h>
#include <signal.h>

#include <string>
#include <iostream>

#include "db.h"
#include "common.h"
#include "handlers.h"

#define LISTENQ 1024
#define MAXADDR 20

//Client should send the size of packet first, 
//and server will expect the packet according to the size.
#define MAXSIZELEN 8

using namespace std;

static int SERV_PORT = 8083;

void *thr_start(void *arg);

static void init_db();
static void destroy_db();
static string child_recv(int sockfd, int *err);
static string child_distribute(string request);
static int child_send(int sockfd, const string &data);

int main(int argc, char **argv)
{
    if (argc == 2)
    {
        char *endptr = NULL;
        SERV_PORT = strtol(argv[1], &endptr, 10);
        if (*endptr != '\0')
        {
            perror("Port FORMAT error");
            exit(EXIT_FAILURE);
        }
        
    }

    init_db();
    int ret = 0;

    int listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (listen_fd < 0)
    {
        perror("mainloop: socket failed");
        exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
    }

    ret = listen(listen_fd, LISTENQ);

    if (ret < 0)
    {
        perror("mainloop: listen failed");
        exit(EXIT_FAILURE);
    }
    
    //struct sockaddr_in cliaddr;
    //socklen_t len;
    int flag = 1;
    while (flag)
    {

        //int connfd = accept(listen_fd, (struct sockaddr *) &cliaddr, &len);
        
        int *p_connfd = (int *) malloc(sizeof(int));

        *p_connfd = accept(listen_fd, NULL, NULL);

        
        if (*p_connfd < 0)
        {
            perror("mainloop: accept failed");
        }

        pthread_t pthread_id;

        ret = pthread_create(&pthread_id, NULL, thr_start, (void *) p_connfd);

        if (ret != 0)
        {
            cerr << "pthread create failed" << endl;
            cerr.flush();
        }

        //If child thread is not detached from the main pthread,
        //program will occur memory leak. 
        //Child can exit by itself.
        pthread_detach(pthread_id);
        
    }

    destroy_db();

    exit(EXIT_SUCCESS);

}

void *
thr_start(void *fd)
{

    sigset_t signal_mask;
    sigemptyset (&signal_mask);
    sigaddset (&signal_mask, SIGPIPE);
    int rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
    
    if (rc != 0)
    {
            printf("block sigpipe error\n");
    }
    int connfd = *(int *) fd;
    int thr_ret = 0;
    free(fd);

    cerr << "Accept new client" << endl;
    cerr.flush();

    while (!thr_ret)
    {

        string str_body = child_recv(connfd, &thr_ret);

        if (thr_ret != 0)
        {
            break;
        }

        string result = child_distribute(str_body);

        thr_ret = child_send(connfd, result);
    }

    close(connfd);
    
    cerr << "client is gone" << endl;
    cerr.flush();
    pthread_exit(NULL);
}

static 
string 
child_recv(int sockfd, int *err)
{
    *err = 0;
    string request = "";

    //The reason why many users can't get the correct resopnses is 
    //that the buffer is static in the previous version.
    //Wed May  2 17:26:34 CST 2012
    //
    char buffer[MAXBUF];

    size_t nread = MAXSIZELEN;
    size_t size_to_read = MAXSIZELEN;
    while (size_to_read > 0)
    {
        nread = read(sockfd, 
                buffer + MAXSIZELEN - size_to_read, 
                size_to_read);
        if (nread < 0 && errno == EINTR)
        {
            continue;
        } else if (nread <= 0) {
            break;
        }
        size_to_read -= nread;
    }
    buffer[MAXSIZELEN] = '\0';

    char size[MAXBUF];
    snprintf(size, sizeof(size), 
            "%lu has been read on buffersize", 
            MAXSIZELEN - size_to_read);
    cerr << size << endl;
    cerr.flush();

    if (nread <= 0)
    {
        *err = 1;
        return "";
    }

    size_to_read = atoi(buffer);

    cerr << "buffer begin:" << endl;
    cerr << buffer << endl;
    cerr << "buffer end;" << endl;
    cerr.flush();

    int cur_size = 0;

    memset(buffer, 0, sizeof(buffer));

    while (size_to_read > 0)
    {
        cur_size = read(sockfd, buffer, size_to_read);
        cerr << cur_size << " has been read" << endl;
        cerr << buffer << endl;
        cerr << "output stop" << endl;
        cerr.flush();
        if (cur_size < 0 && errno == EINTR)
        {
            continue;
        } else if (cur_size <= 0) {
            request = "500 SOCKETREAD ERROR\r\n\r\n";
            cerr << "socket read fail" << endl;
            cerr.flush();
            *err = 1;
            break;
        }
        
        buffer[cur_size] = '\0';
        string content = buffer;
        request += content;
        size_to_read -= cur_size;
    }
    cerr << "out of read loop" << endl;
    cerr.flush();


    return request;
}

static 
string 
child_distribute(string request)
{
    string result;
    size_t loc = request.find_first_of(" \r\n");
    string command = request.substr(0, loc);
    //cout << request << endl;
    //cout.flush();

    if (command.find("LOGIN") != string::npos){
        cout << "enter handle" << endl;
        cout.flush();
        result = handle_login(request);
        //cout << result << endl;
        //cout.flush();

    } else if (command.find("LSTET") != string::npos){
        cout << "enter LSTET" << endl;
        cout.flush();
        result = handle_LSTET(request);

    } else if (command.find("LSTES") != string::npos){
        cout << "enter LSTES" << endl;
        cout.flush();
        result = handle_LSTES(request);
    } else if (command.find("MEINF") != string::npos){
        cout << "enter MEINF" << endl;
        cout.flush();
        result = handle_MEINF(request);
    }else if (command.find("EINF") != string::npos){
        cout << "enter EINF" << endl;
        cout.flush();
        result = handle_EINF(request);

    } else if (command.find("UPANS") != string::npos){
        cout << "enter UPANS" << endl;
        cout.flush();
        result = handle_UPANS(request);
    } else if (command.find("ADDE") != string::npos){
        cout << "enter ADDE" << endl;
        cout.flush();
        result = handle_ADDE(request);

    } else if (command.find("MPSTA") != string::npos){
        cout << "enter MPSTA" << endl;
        cout.flush();
        result = handle_MPSTA(request);
    } else if (command.find("ADDQ") != string::npos){
        cout << "enter ADDQ" << endl;
        cout.flush();
        result = handle_ADDQ(request);
    } else if (command.find("MQINF") != string::npos){
        cout << "enter MQINF" << endl;
        cout.flush();
        result = handle_MQINF(request);
    } else if (command.find("DELE") != string::npos){
        cout << "enter DELE" << endl;
        cout.flush();
        result = handle_DELE(request);
    } else if (command.find("DELQ") != string::npos){
        cout << "enter DELQ" << endl;
        cout.flush();
        result = handle_DELQ(request);
    } else if (command.find("LSTQ") != string::npos){
        cout << "enter LSTQ" << endl;
        cout.flush();
        result = handle_LSTQ(request);
    } else if (command.find("GETQNUM") != string::npos){
        cout << "enter GETQNUM" << endl;
        cout.flush();
        result = handle_GETQNUM(request);
    } else if (command.find("ERES") != string::npos) {
        cout << "enter ERES" << endl;
        cout.flush();
        result = handle_ERES(request);
    } else if (command.find("STARTE") != string::npos) {
        cout << "enter STARTE" << endl;
        cout.flush();
        result = handle_STARTE(request);
    } else if (command.find("NEXTQ") != string::npos){
        cout << "enter NEXTQ" << endl;
        cout.flush();
        result = handle_NEXTQ(request);
    } else if (command.find("LSTERS") != string::npos){
        cout << "enter LSTERS" << endl;
        cout.flush();
        result = handle_LSTERS(request);
    } else if (command.find("SEDT") != string::npos){
        cout << "enter SEDT" << endl;
        cout.flush();
        result = handle_SEDT(request);
    } else if (command.find("MUSRINF") != string::npos){
        cout << "enter MUSRINF" << endl;
        cout.flush();
        result = handle_MUSRINF(request);
    } else if (command.find("USRINF") != string::npos){
        cout << "enter USRINF" << endl;
        cout.flush();
        result = handle_USRINF(request);

    } else if (command.find("PWDCHG") != string::npos){
        cout << "enter PWDCHG" << endl;
        cout.flush();
        result = handle_PWDCHG(request);
    } else if (command.find("PWDRST") != string::npos){
        cout << "enter PWDRST" << endl;
        cout.flush();
        result = handle_PWDRST(request);
    } else if (command.find("USRADD") != string::npos){
        cout << "enter USRADD" << endl;
        cout.flush();
        result = handle_USRADD(request);
    } else if (command.find("USRDEL") != string::npos){
        cout << "enter USRDEL" << endl;
        cout.flush();
        result = handle_USRDEL(request);
    } else {
        result = "500 DISTRUBUTE COMMAND NOT FOUND\r\n\r\n";
        cerr << request << endl
            << result << endl;
        cerr.flush();
    }

    return result;
}

static 
int 
child_send(int sockfd,const string &data)
{
    size_t size = data.size();
    char buf_size[MAXSIZELEN];
    char buffer[MAXBUF];
    memmove(buffer, data.c_str(), size);
    buffer[size] = '\0';

    snprintf(buf_size, sizeof(buf_size), "%lu", size + 1);
    write(sockfd, buf_size, sizeof(buf_size));

    cerr << size + 1 << ":" << buf_size << endl 
        << buffer << endl;
    cerr.flush();


    size_t len_write;
    size_t len_start = 0;
    while ( len_start < size + 1)
    {
        len_write = write(sockfd, 
                buffer + len_start, size + 1 - len_start);
        if (len_write < 0)
        {
            if (errno == EINTR)
            {
                continue ;
            }
            //perror("mainloop: write fail");
            break;
        }
        len_start += len_write;
    }

    if (len_write < 0)
    {
        return -1;
    } else {

        return 0;
    }
}

static 
void 
init_db()
{
    struct DBConns *ptr = 
        (struct DBConns *) mmap(NULL, 
                sizeof(struct DBConns), 
                PROT_READ | PROT_WRITE, 
                MAP_SHARED | MAP_ANON,
                -1, 
                0);
    ptr->count = 0;
    sem_init(&ptr->mutex, 1, 1);

    DB::p_dbconns = ptr;

    sem_wait(&ptr->mutex);

    for (unsigned int i = 0; i < DB::MAX_CON; ++i)
    {
        int count = 0;
        do {
            ptr->conns[i] = PQconnectdb(DB::conninfo);
            count++;
        } while (PQstatus(ptr->conns[i]) != CONNECTION_OK && count < 3);
        if (count == 3)
        {
            cerr << "PQ error: No more connections can be established" << endl;
            cerr.flush();
            exit(EXIT_FAILURE);
            break;
        }

        ptr->used[i] = 0;
        ptr->count = i;
    }

    sem_init(&ptr->sem_conns, 1, ptr->count);
    sem_post(&ptr->mutex);

    std::cout << ptr->count << " connections has been established to Database" << std::endl;
    std::cout.flush();


    return ;
}

static 
void
destroy_db()
{
    struct DBConns *ptr = DB::p_dbconns;
    sem_wait(&ptr->mutex);

    for (unsigned int i = ptr->count - 1; i >= 0; --i)
    {
        PQfinish(ptr->conns[i]);
        ptr->count --;
    }
    sem_post(&ptr->mutex);
    
    return ;
}
