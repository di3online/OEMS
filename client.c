#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXSIZELEN 8

#define SERV_PORT 8083
int main(int argc, char **argv)
{
    assert(argc == 4);
    int times = atoi(argv[1]);
    int all_times = atoi(argv[2]);;
    char buffer_recv[1024];
    char buffer_send[1024];
    char bufrecv_size[MAXSIZELEN];
    char bufsend_size[MAXSIZELEN];
    size_t len_to_send;
    int ret;

    //FILE *fin = fopen("test/login.in", "r");
    //FILE *fin = fopen("test/login_teacher.in", "r");
    //FILE *fin = fopen("test/lstet.in", "r");
    //FILE *fin = fopen("test/einf.in", "r");
    //FILE *fin = fopen("test/upans.in", "r");
    //FILE *fin = fopen("test/lstes.in", "r");

    FILE *fin = fopen(argv[3], "r");

    assert(fin);

    fseek(fin, 0L, SEEK_SET); 
    len_to_send = fread(buffer_send, 1, sizeof(buffer_send), fin);

    fclose(fin);

    memset(bufsend_size, 0, sizeof(bufsend_size));
    snprintf(bufsend_size, sizeof(bufsend_size), "%lu", len_to_send);



    int count = 0;
    printf("%d \n", 500 / times);
    fflush(stdout);
    int i;
    int child_count = 0;
    for (i = 0; i < all_times / times; i++)
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
                size_t len_write;
                len_write = write(connfd, bufsend_size, sizeof(bufsend_size));
                printf("write %lu on bufsend_size\n", len_write);
                len_write = write(connfd, buffer_send, len_to_send);
                printf("write %ld\n", len_write);


                size_t len_recv;
                len_recv = read(connfd, bufrecv_size, sizeof(bufrecv_size));
                printf("read %lu on bufrecv_size\nsize: %s\n", len_recv, bufrecv_size);

                int len_to_recv = atoi(bufrecv_size);

                buffer_recv[0] = '\0';

                len_recv = read(connfd, buffer_recv, len_to_recv);

                if (buffer_recv[0] == '2' && buffer_recv[1] == '0' && buffer_recv[2] == '0')
                {
                    printf("read good\n");
                } else {
                    printf("read bad\n");
                }
                printf("read %lu on body\n", len_recv);
                printf("%s\n", buffer_recv);
                fflush(stdout);

                //puts(write_buffer);
            }
            close(connfd);
            exit(0);
        }


    }
    
    return 0;
}
