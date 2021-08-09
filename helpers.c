#include "helpers.h"

/**
 * reads and echoes text from client until client closes connection
 */
void echo(int connfd) {
    size_t n;
    char buf[MAXLINE];
    //printf("connfd in echo %d\n", connfd);
    while ((n = read(connfd, buf, MAXLINE)) != 0) {
        printf("server received %d bytes \n", (int)n);
        //printf("connfd in echo2 %d\n", connfd);
        write(connfd, buf, n);
        memset(buf, 0, MAXLINE);
    }
    printf("client on process %d disconnected\n", getpid());
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

//Misc helpers
//Unix style error handling to display message
void unix_error(char *msg) {
    fprintf(stderr, "%s:%d: %s\n", msg, errno, strerror(errno));
    exit(-1);
}

//Error-handling wrapper for fork
pid_t Fork(void) {
    pid_t pid;

    if((pid = fork()) < 0)
        unix_error("fork error");
    return pid;
}

//Signal - portable signal handling on Posix-compliant systems
handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* Block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* Restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
	    unix_error("Signal error");
    return (old_action.sa_handler);
}

//Close wrapper
void Close(int fd) {
    int rc;
    
    if ((rc = close(fd)) < 0)
        unix_error("close error");
}