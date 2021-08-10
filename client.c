/**
 * echo client
 * mainly from http://csapp.cs.cmu.edu/3e/ics3/code/netp/echoclient.c
 * to compile:
 * gcc -Wall -pthread client.c helpers.c -o client
 * to use:
 * ./client 127.0.0.1 <port #>
 */

#include "helpers.h"

//handler for SIGPIPE
void sigpipe_handler(int sig) {
    printf("Server closed\n");
    fflush(stdout);
}

//thread routine for outputting
void *thread(void *clientfdp);

int main(int argc, char **argv) 
{
    int *clientfdp, clientfd;
    char *host, *port, buf[MAXLINE];
    int n;
    pthread_t tid;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = argv[2];

    //handling if server closes connection
    Signal(SIGPIPE, sigpipe_handler);
    
    clientfdp = Malloc(sizeof(int));
    *clientfdp = open_clientfd(host, port);
    memset(buf, 0, MAXLINE);
    Pthread_create(&tid, NULL, thread, clientfdp);
    
    clientfd = *clientfdp;

    while (fgets(buf, MAXLINE, stdin) != NULL) {
        if ((n = write(clientfd, buf, strlen(buf))) <= 0) {
            fprintf(stderr, "ERROR writing to socket: %s\n",strerror(errno));
            exit(-1);
        } else {
            printf("n = %d, Sent to server (len %ld): %s\n",n, strlen(buf), buf);
        }
    }
    Close(clientfd); 
    exit(0);
}

void *thread(void *clientfdp) {
    int n;
    char buf[MAXLINE];
    //set for IO multiplexing
    fd_set read_set, ready_set;

    int clientfd = *((int *)(clientfdp));
    Pthread_detach(pthread_self());
    free(clientfdp);
    //initializing set
    FD_ZERO(&read_set);
    FD_SET(clientfd, &read_set);
    //checking
    while (1) {
        ready_set = read_set;
        Select(clientfd+1, &ready_set, NULL, NULL, NULL);
        if (FD_ISSET(clientfd, &ready_set)) {
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
    }
}