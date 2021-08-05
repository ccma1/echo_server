/**
 * a simple blocking echo server
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

/*constants */
#define MAXLINE 8194
#define LISTENQ 1024

//sockaddr def
typedef struct sockaddr SA; 

//echo 
void echo(int connfd);
//returns a listening socket ready to receive conn requests on port
int open_listenfd(char *port);

int main(int argc, char **argv){
    int listenfd, connfd; //listening and connection file descriptor
    socklen_t clientlen; //specifies the size of client's addr
    struct sockaddr_storage clientaddr; //socket addr of client
    char client_hostname[MAXLINE], client_port[MAXLINE];
    int rc;

    listenfd = open_listenfd(argv[1]);
    while(1) {
        clientlen = sizeof(struct sockaddr_storage);
        //wait for connection request from client on listenfd
        if ((connfd = accept(listenfd, (SA*)&clientaddr, &clientlen) < 0)) {
            fprintf(stderr, "accept error: %s\n", strerror(errno));
            exit(-1);
        }
        printf("connfd in main %d\n", connfd);
        if ((rc = getnameinfo((SA*)&clientaddr, clientlen,client_hostname, MAXLINE, client_port, MAXLINE, 0)) < 0) {
                        fprintf(stderr, "getnameinfo error: %s\n", gai_strerror(rc));
        }
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        echo(connfd);
        close(connfd);
    }
    exit(0);
}

/**
 * reads and echoes text from client until client closes connection
 */
void echo(int connfd) {
    size_t n;
    char buf[MAXLINE];
    printf("connfd in echo %d\n", connfd);
    while ((n = read(connfd, buf, MAXLINE)) != 0) {
        printf("server received %d bytes \n", (int)n);
        printf("connfd in echo2 %d\n", connfd);
        write(connfd, buf, n);
    }
    close(connfd);
}

/**
 * opens and returns a listening socket on port
 */
int open_listenfd(char *port) {
    struct addrinfo hints, *listp, *p;
    int listenfd, rc, optval=1;

    //gets linked list of addrinfo structs for host and service
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;
    if ((rc = getaddrinfo(NULL, port, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo fails port %s: %s\n", port, gai_strerror(rc));
        return -2;
    }

    //walk list for a socket that can be binded to
    for (p = listp; p; p = p->ai_next) {
        //create socket descriptor
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue; //socket failed and try
        
        /* Eliminates "Address already in use" error from bind */
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,    //line:netp:csapp:setsockopt
                   (const void *)&optval , sizeof(int));
        
        //bind the descriptor to the address
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;
        if (close(listenfd) < 0) {
            fprintf(stderr, "open_listenfd close failed: %s\n", strerror(errno));
            return -1;
        }
    }

    //clean up getaddrinfo
    freeaddrinfo(listp);
    //if no address worked
    if (!p) return -1;

    //make p a listening socket ready to accept connection requests
    if (listen(listenfd, LISTENQ) < 0) {
        fprintf(stderr, "listen error: %s\n", strerror(errno));
        return -1;
    }
    return listenfd;
}   