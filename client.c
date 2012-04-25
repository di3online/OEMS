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
    int i;
    for (i = 0; i < 1000; i++)
    {
    
    int ret;
    FILE *fin = fopen("login.in", "r");
    assert(fin);

    int connfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    assert(connfd != -1);

    struct sockaddr_in servaddr;

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    ret = inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    
    assert(ret > 0);

    ret = connect(connfd, (const struct sockaddr *) &servaddr, sizeof(servaddr));

    assert(ret >= 0);

    char buffer[1024];
    char buf_size[MAXSIZELEN];
    while (!feof(fin))
    {
        memset(buf_size, 0, sizeof(buf_size));
        size_t len_read = fread(buffer, 1, sizeof(buffer), fin);
        snprintf(buf_size, sizeof(buf_size), "%lu", len_read);
        write(connfd, buf_size, sizeof(buf_size));
        size_t len_write = write(connfd, buffer, len_read);

        read(connfd, buf_size, sizeof(buf_size));
        len_read = atoi(buf_size);

        read(connfd, buffer, len_read);

        puts(buffer);
    }

    close(connfd);
    fclose(fin);

    }
    
    return 0;
}
