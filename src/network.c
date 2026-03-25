#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <error.h>
#include "network.h"
#include "debug.h"
int srv;
// Send formatted string to a socket
int vsendf(int socket, char* fmt, va_list ap)
{
    size_t size;
    int n=254;
    char *m=NULL;
    int err=0;
    //debug("Sending message to %d:\n\t",socket);
    //vdebug(fmt, ap);
    //debug("\n");
    do
    {
        size=n+1;
        m=(char*) realloc(m,(size_t) size);
        n=vsnprintf(m,size,fmt,ap);
        va_end(ap);
    }
    while(n>size);
    err=write(socket,m,strlen(m)+1);
    free(m);
    //signal(SIGPIPE,SIG_DFL);
    if (err<0) return(1); else return(0);
}


int sendf(int socket, char* fmt, ...)
{
    int err;
    va_list ap;
    va_start(ap,fmt);
    err=vsendf(socket,fmt,ap);
    va_end(ap);
    return(err);
}


// Connect to a server
int connect2server(char *srv_name, uint16_t port)
{
    int srv=0;
    struct sockaddr_in server;
    struct hostent  *gep;
    int p;
    p=port;
    while(1)
    {
        server.sin_family=AF_INET;
        server.sin_port=htons(p);
        gep=gethostbyname(srv_name);
        if (gep==NULL)
            error(1,h_errno,"gethostbyname()");
        server.sin_addr = *(struct in_addr *)gep-> h_addr;
        srv=socket(PF_INET, SOCK_STREAM, 0);
        if (srv <0 )
            error(1,errno,"socket()");
        if (connect(srv, (struct sockaddr *) &server, sizeof(server))<0)
            error(1,errno,"Error connecting to server:"); else
            return(srv);
    }
}


// Get a line from a puffer

char* getAline(char **tostr, char *from, int *end)
{
    debug(7,"getAline called\n");
    int i;
    for(i=0;i<*end;i++)
    {
        if(from[i]==DELIM)
        {
            *tostr=(char *) malloc((i+1)*sizeof(char));
            if (!*tostr) error(1,errno,"malloc()");
            memcpy(*tostr,from,i+1);
            memmove(from,&from[i+1],*end-i-1);
            *end=*end-i-1;
            from[*end]='\0';
            debug(7,"Got line: %s ::: new pos: %d ::: puffer: %c\n",*tostr,*end,from[0]);
            return(*tostr);
        }
    }
    if(*end>=BS) error(1,0,"Buffer is full!!");
    debug(7,"getAline returns NULL\n");
    return(NULL);
}
