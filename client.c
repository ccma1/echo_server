/**
 * echo client
 * mainly from http://csapp.cs.cmu.edu/3e/ics3/code/netp/echoclient.c
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <errno.h>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

/**constants */
#define MAXLINE 8194

//opens connection to server at (hostname, port) and returns socket descriptor
int open_clientfd(char *hostname, char *port);

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

/*
 * open_clientfd - Open connection to server at <hostname, port> and
 *     return a socket descriptor ready for reading and writing.
*/
int open_clientfd(char *hostname, char *port) {
    int clientfd, rc;
    struct addrinfo hints, *listp, *p;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;  
    hints.ai_flags = AI_NUMERICSERV;  
    hints.ai_flags |= AI_ADDRCONFIG;  
    if ((rc = getaddrinfo(hostname, port, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo failed (%s:%s): %s\n", hostname, port, gai_strerror(rc));
        return -2;
    }
  
    /* Walk the list for one that we can successfully connect to */
    for (p = listp; p; p = p->ai_next) {
        /* Create a socket descriptor */
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) 
            continue; /* Socket failed, try the next */

        /* Connect to the server */
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1) 
            break; /* Success */
        if (close(clientfd) < 0) { /* Connect failed, try another */  
            fprintf(stderr, "open_clientfd: close failed: %s\n", strerror(errno));
            return -1;
        } 
    } 

    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* All connects failed */
        return -1;
    else    /* The last connect succeeded */
        return clientfd;
}