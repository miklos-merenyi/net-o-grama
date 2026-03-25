//same as client, but without sound
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdarg.h>
#include <string.h>
#include <ncurses.h>
#include <signal.h>
#include "network.h"
#include "debug.h"
#include "linked.h"
#include "engine.h"
char srvname[50];
extern int port;
extern char usrname[9];
void initConnection(char* server, int port) ;
void heartBeat();
void recv_msg();
void keyPressed(int key);
struct gamer
{
    char name[9];
    int score;
    int state;
};
