#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <errno.h>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <signal.h>
#include <sys/wait.h>

#include <pthread.h>

/*constants */
#define MAXLINE 8194
#define LISTENQ 1024

//sockaddr def
typedef struct sockaddr SA; 

//typedef void (*sighandler_t)(int);
typedef void handler_t(int);

//server functions
//echo function to write back to client
void echo(int connfd);
//returns a listening socket ready to receive conn requests on port
int open_listenfd(char *port);

//client functions
//opens connection to server at (hostname, port) and returns socket descriptor
int open_clientfd(char *hostname, char *port);

//Misc helpers
//Unix-style error handling
void unix_error(char *msg);

//Posix-style error handling
void posix_error(int code, char *msg);

//fork error handling wrapper
pid_t Fork(void);

//Signal - portable signal handling
handler_t *Signal(int signum, handler_t *handler);

//Close wrapper
void Close(int fd);

//Malloc wrapper
void *Malloc(size_t size);

//Pthread create wrapper
void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp, void * (*routine)(void *), void *argp);

//Pthread detatch wrapper
void Pthread_detach(pthread_t tid);

#endif