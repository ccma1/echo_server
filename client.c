/**
 * echo client
 * mainly from http://csapp.cs.cmu.edu/3e/ics3/code/netp/echoclient.c
 * to compile:
 * gcc -Wall client.c helpers.c -o client
 * to use:
 * ./client 127.0.0.1 <port #>
 */

#include "helpers.h"

int main(int argc, char **argv) 
{
    int clientfd;
    char *host, *port, buf[MAXLINE];
    int n;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = argv[2];

    clientfd = open_clientfd(host, port);
    memset(buf, 0, MAXLINE);

    while (fgets(buf, MAXLINE, stdin) != NULL) {
        if ((n = write(clientfd, buf, strlen(buf))) <= 0) {
            fprintf(stderr, "ERROR writing to socket: %s\n",strerror(errno));
            exit(-1);
        } else {
            printf("n = %d, Sent to server (len %ld): %s\n",n, strlen(buf), buf);
        }
        //memset(buf, 0, MAXLINE);
        if ((n = read(clientfd, buf, strlen(buf))) <= 0) {
            fprintf(stderr, "ERROR reading from socket: %s\n",strerror(errno));
            exit(-1);
        } else {
            printf("Echo from server: %s\n", buf);
        }
    }
    close(clientfd); 
    exit(0);
}