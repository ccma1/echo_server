/**
 * Concurrent echo server based on pthreads
 * mostly based on:
 * http://csapp.cs.cmu.edu/3e/ics3/code/src/csapp.c
 * to compile:
 * gcc -Wall -pthread echo_server.c helpers.c -o echo_server
 * to run:
 * ./echo_server <port #>
*/

#include "helpers.h"

//thread routine
void *thread(void *connfdp);

int main(int argc, char **argv){
    int listenfd, *connfdp; //listening and connection file descriptor
    socklen_t clientlen; //specifies the size of client's addr
    struct sockaddr_storage clientaddr; //socket addr of client
    char client_hostname[MAXLINE], client_port[MAXLINE]; //client hostname and port
    int rc;
    pthread_t tid;

    listenfd = open_listenfd(argv[1]);
    while(1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = Malloc(sizeof(int));
        //Accept - wait for connection request from client on listenfd
        if ((*connfdp = accept(listenfd, (SA*)&clientaddr, &clientlen)) < 0) 
            unix_error("accept error");
        //get client info
        if ((rc = getnameinfo((SA*)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0)) < 0) {
                        fprintf(stderr, "getnameinfo error: %s\n", gai_strerror(rc));
                        exit(-1);
        }
        printf("Connected to (%s, %s) on fd %d\n", client_hostname, client_port, *connfdp);
        //clear hostname and port info
        memset(client_hostname, 0, MAXLINE);
        memset(client_port, 0, MAXLINE);
        //spawn new thread
        Pthread_create(&tid, NULL, thread, connfdp);
    }
    exit(0);
}

void *thread(void *connfdp) {
    int connfd = *((int*)(connfdp));
    Pthread_detach(pthread_self());
    free(connfdp);
    echo(connfd);
    Close(connfd);
    return NULL;
}