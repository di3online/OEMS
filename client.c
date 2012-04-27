#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXSIZELEN 8

#define SERV_PORT 8083
int main()
{
    char read_buffer[1024];
    char write_buffer[1024];
    char buf_size[MAXSIZELEN];
    char bufsize_read[MAXSIZELEN];
    int ret;
    FILE *fin = fopen("login.in", "r");
    assert(fin);

    fseek(fin, 0L, SEEK_SET); 
    size_t len_read = fread(read_buffer, 1, sizeof(read_buffer), fin);

    size_t len_send = len_read;

    memset(bufsize_read, 0, sizeof(bufsize_read));
    snprintf(bufsize_read, sizeof(bufsize_read), "%lu", len_read);



    int count = 0;
    int times = 5;
    printf("%d \n", 500 / times);
    fflush(stdout);
    int i;
    int child_count = 0;
    for (i = 0; i < 5000 / times; i++)
    {
        printf("child : %d\n", child_count++);
        fflush(stdout);
        if (fork() == 0)
        {
            
            int connfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

            assert(connfd != -1);

            struct sockaddr_in servaddr;

            servaddr.sin_family = AF_INET;
            servaddr.sin_port = htons(SERV_PORT);
            ret = inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
            
            assert(ret > 0);

            ret = connect(connfd, (const struct sockaddr *) &servaddr, sizeof(servaddr));

            assert(ret >= 0);
            
            int j = 0;
            for (j = 0; j < times; j++)
            {
                printf("%d:\n", count); 

                count++;
                write(connfd, bufsize_read, sizeof(bufsize_read));
                size_t len_write = write(connfd, read_buffer, len_send);
                printf("write %lu\n", len_write);

                read(connfd, buf_size, sizeof(buf_size));
                len_read = atoi(buf_size);

                read(connfd, write_buffer, len_read);

                //puts(write_buffer);
            }
            close(connfd);
            exit(0);
        }


    }
    
    fclose(fin);
    return 0;
}
