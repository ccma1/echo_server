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
//echo function for writing to all connected clients
void echo_all(int connfd);
//add connection file descriptor to client_fd array
void add_connfd(int connfd);
//remove client fd from client_fd array
void remove_connfd(int connfd);
//send_to_all sends msg in buf to all clients in connfds
void send_to_all(void);

char buf[MAXLINE]; //global variable for writing to clients
int connfds[LISTENQ]; //array of client fds
sem_t connfds_mutex, buf_mutex; //semaphore for buffer and file descriptors

int main(int argc, char **argv){
    int listenfd, *connfdp; //listening and connection file descriptor
    socklen_t clientlen; //specifies the size of client's addr
    struct sockaddr_storage clientaddr; //socket addr of client
    char client_hostname[MAXLINE], client_port[MAXLINE]; //client hostname and port
    int rc;
    pthread_t tid;

    Sem_init(&buf_mutex, 0, 1);
    Sem_init(&connfds_mutex, 0, 1);

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
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        //clear hostname and port info
        memset(client_hostname, 0, MAXLINE);
        memset(client_port, 0, MAXLINE);
        //mark connfd as used and increment count
        P(&connfds_mutex);
        add_connfd(*connfdp);
        V(&connfds_mutex);
        //spawn new thread
        Pthread_create(&tid, NULL, thread, connfdp);
    }
    exit(0);
}

void *thread(void *connfdp) {
    int connfd = *((int*)(connfdp));
    Pthread_detach(pthread_self());
    free(connfdp);
    echo_all(connfd);
    Close(connfd);
    P(&connfds_mutex);
    remove_connfd(connfd);
    V(&connfds_mutex);
    return NULL;
}

//writes message in buf to all clients
void echo_all(int connfd) {
    size_t n;
    char curr_buf[MAXLINE];
    while ((n = read(connfd, curr_buf, MAXLINE)) != 0) {
        printf("server received %d bytes\n", (int)n);
        P(&buf_mutex);
        P(&connfds_mutex);
        strncpy(buf, curr_buf, MAXLINE);
        send_to_all();
        memset(buf, 0, MAXLINE);
        V(&buf_mutex);
        V(&connfds_mutex);
        memset(curr_buf, 0, MAXLINE);
    }
}

void send_to_all(void) {
    for (int i = 0; i < LISTENQ; i++) {
        if (connfds[i] != 0) {
            write(connfds[i], buf, MAXLINE);
        }
    }
}

//add connfd to connfds array
void add_connfd(int connfd) {
    for (int i = 0; i < LISTENQ; i++) {
        if (connfds[i] == 0) {
            connfds[i] = connfd;
            return;
        }
    }
    unix_error("add_conn error - exceed max number of clients");
}

//remove connfd from connfds array
void remove_connfd(int connfd) {
    for (int i = 0; i < LISTENQ; i++) {
        if (connfds[i] == connfd) {
            connfds[i] = 0;
            return;
        }
    }
    unix_error("remove_connfd error - given connfd doesn't exist");
}