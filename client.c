/**
 * echo client
 * mainly from http://csapp.cs.cmu.edu/3e/ics3/code/netp/echoclient.c
 * to compile:
 * gcc -Wall client.c helpers.c -o client
 * to use:
 * ./client 127.0.0.1 <port #>
 */

#include "helpers.h"

//handler for SIGPIPE
void sigpipe_handler(int sig) {
    printf("Server closed\n");
    fflush(stdout);
}

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

    //handling if server closes connection
    Signal(SIGPIPE, sigpipe_handler);

    clientfd = open_clientfd(host, port);
    memset(buf, 0, MAXLINE);

    while (fgets(buf, MAXLINE, stdin) != NULL) {
        if ((n = write(clientfd, buf, strlen(buf))) <= 0) {
            fprintf(stderr, "ERROR writing to socket: %s\n",strerror(errno));
            exit(-1);
        } else {
            printf("n = %d, Sent to server (len %ld): %s\n",n, strlen(buf), buf);
        }
        //write(clientfd, "hello", 6);
        //memset(buf, 0, MAXLINE);
        if ((n = read(clientfd, buf, MAXLINE)) < 0) {
            fprintf(stderr, "ERROR reading from socket: %s\n",strerror(errno));
            exit(-1);
        } else if (n == 0) {
            fprintf(stderr, "Server disconnected\n");
            exit(-1);
        } else {
            printf("Received %d bytes, Echo from server: (%s)\n", n, buf);
        }
    }
    Close(clientfd); 
    exit(0);
}