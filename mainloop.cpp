#include <cstdio>
#include <cstdlib>
#include <cstring>

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

const static int SERV_PORT = 8083;

void *thr_start(void *arg);

static void init_db();
static string child_recv(int sockfd, int *err);
static string child_distribute(const string &request);
static int child_send(int sockfd, const string &data);

int main()
{
    init_db();
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
    
    //struct sockaddr_in cliaddr;
    //socklen_t len;
    while (1)
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


        /*
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

        }*/


        
    }

    return 0;

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
    pthread_exit((void *) 0);
}

static 
string 
child_recv(int sockfd, int *err)
{
    *err = 0;
    string request = "";
    static char buffer[MAXBUF];

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

    char size[MAXSIZELEN];
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
        request += buffer;
        size_to_read -= cur_size;
    }
    cerr << "out of read loop" << endl;
    cerr.flush();

    return request;
}

static 
string 
child_distribute(const string &request)
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
        } while (ptr->conns[i] == NULL && count < 3);
        if (count == 3)
        {
            break;
        }

        ptr->used[i] = 0;
        ptr->count = i;
    }

    sem_init(&ptr->sem_conns, 1, ptr->count);
    sem_post(&ptr->mutex);

    return ;
}
