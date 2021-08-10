/**
 * prethreaded concurrent echo server
 * mostly based on:
 * http://csapp.cs.cmu.edu/3e/ics3/code/src/csapp.c
 * to compile:
 * gcc -Wall -pthread echo_server.c helpers.c sbuf.c -o echo_server
 * to run:
 * ./echo_server <port #>
*/

#include "helpers.h"
#include "sbuf.h"
#define NTHREADS 2
#define SBUFSIZE 16

//thread routine
void *thread(void *connfdp);
//echos to client
void echo_worker(int connfd);

//shared buffer
sbuf_t sbuf;


int main(int argc, char **argv){
    int listenfd, connfd; //listening and connection file descriptor
    socklen_t clientlen; //specifies the size of client's addr
    struct sockaddr_storage clientaddr; //socket addr of client
    char client_hostname[MAXLINE], client_port[MAXLINE]; //client hostname and port
    int rc;
    pthread_t tid;

    listenfd = open_listenfd(argv[1]);
    //initialize buffer with SBUFSIZE slots
    sbuf_init(&sbuf, SBUFSIZE);
    //create thread pool
    for (int i = 0; i < NTHREADS; i++)
        Pthread_create(&tid, NULL, thread, NULL);

    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        //Accept - wait for connection request from client on listenfd
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
        sbuf_insert(&sbuf, connfd);
    }
    sbuf_deinit(&sbuf);
    exit(0);
}

void *thread(void *connfdp) {
    Pthread_detach(pthread_self());
    while (1) {
        int connfd = sbuf_remove(&sbuf);
        echo_worker(connfd);
        Close(connfd);
    }
    return NULL;
}

void echo_worker(int connfd) {
    int n;
    char buf[MAXLINE];
    while ((n = read(connfd, buf, MAXLINE)) != 0) {
        printf("Server received %d bytes\n", n);
        write(connfd, buf, n);
        memset(buf, 0, MAXLINE);
    }
    printf("Client with file descriptor %d disconnected\n", connfd);
}