/**
 * a simple blocking echo server
 * mostly based on:
 * http://csapp.cs.cmu.edu/3e/ics3/code/src/csapp.c
 * http://csapp.cs.cmu.edu/3e/ics3/code/netp/echo.c
 * http://csapp.cs.cmu.edu/3e/ics3/code/netp/echoserveri.c
 * to compile:
 * gcc -Wall echo_server.c helpers.c -o echo_server
 * to run:
 * ./echo_server <port #>
*/

#include "helpers.h"

//handler for SIGCHLD
void sigchld_handler(int sig) {
    while (waitpid(-1, 0, WNOHANG) > 0)
        ;
    return;
}

int main(int argc, char **argv){
    int listenfd, connfd; //listening and connection file descriptor
    socklen_t clientlen; //specifies the size of client's addr
    struct sockaddr_storage clientaddr; //socket addr of client
    char client_hostname[MAXLINE], client_port[MAXLINE];
    int rc;

    Signal(SIGCHLD, sigchld_handler);
    listenfd = open_listenfd(argv[1]);
    while(1) {
        clientlen = sizeof(struct sockaddr_storage);
        //wait for connection request from client on listenfd
        if ((connfd = accept(listenfd, (SA*)&clientaddr, &clientlen)) < 0) 
            unix_error("accept error");
        //get client info
        if ((rc = getnameinfo((SA*)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0)) < 0) {
                        fprintf(stderr, "getnameinfo error: %s\n", gai_strerror(rc));
                        exit(-1);
        }
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        //clear hostname and port info
        memset(client_hostname, 0, MAXLINE);
        memset(client_port, 0, MAXLINE);
        if (Fork() == 0) {
            printf("I am child start %d\n", getpid());
            Close(listenfd);
            echo(connfd);
            printf("I am child out %d\n", getpid());
            Close(connfd);
            exit(0);
        }        
        Close(connfd);
    }
    exit(0);
}