#define _GNU_SOURCE
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <glib.h>
#include "network.h"
#include <stdio.h>
#include "debug.h"
#include "linked.h"
#include "dlb.h"
#include "engine.h"
#define PORT 5555
#define GAMETIME 300
#define PROGNAME "nog_srv"

/* Global debug configuration (declared as extern in linked.h). */
int debugLevel = 10;
char *logfile = "./nog_srv.log";

extern struct dlb_node* dlbHead;

struct player
{
    char name[9];
    int id;                      //id during a game
    // socket properties
    int foglalat;
    /* IPv4 string length up to 15 chars + '\0'. */
    char ip[16];
    int port;
    // buffer and pointer
    char buff[1024];
    int p;
    // attributes
    int ready;
    int score;
    struct player* next;
    int alive;                   // for heartbeat
};

struct player* player1=NULL;     // the head of the list of the players
struct node* head=NULL;
char rootWord[10];
int foglalat;

char quit=0;
int gameIsOn=0;
extern int gameTime;
extern int gameStart;
int autoStart=1;                 // the game will start, if there are at least #autoStart players, and all are ready
int timeNow;
int availableTime=GAMETIME;

void removePlayer(struct player* pl2rem);
int _listen(uint16_t p)
{
    int foglalat;
    struct sockaddr_in barki;
    foglalat=socket(PF_INET, SOCK_STREAM, 0);
    if (foglalat <0) error(1,errno,"socket()");
    barki.sin_family=AF_INET;
    barki.sin_addr.s_addr=htonl(INADDR_ANY);
    while(1)
    {
        barki.sin_port=htons(p);
        if ( bind(foglalat, (struct sockaddr *) &barki, sizeof(barki)) <0) error(1,errno,"bind()");
        else
        {
            if (listen(foglalat,1)<0) error(1,errno,"listen()");
            return(foglalat);
        }
    }
}


int acceptClient(int socket)
{
    struct sockaddr_in ugyfel;
    socklen_t meret;
    int uj;
    uj=accept(socket, (struct sockaddr *) &ugyfel, &meret);
    if (uj<0)
    {
        perror("acceptClient()");
        fprintf(stderr,"Error accepting a client!\n");
        return(0);
    }
    else return(uj);
}


//
// send formatted message to all players
//

void send2all(char* msg, ...)
{
    struct player *pl1,*pl;
    va_list ap;
    int err;
    va_start(ap,msg);
    pl=player1;
    while(pl)
    {
        err=vsendf(pl->foglalat,msg,ap);
        pl1=pl;
        pl=pl->next;
        if (err)
        {
            removePlayer(pl1);
        }
    }
    va_end(ap);
}


//
// number of players
//
int numberOfPlayers()
{
    struct player* pl;
    int i=0;
    pl=player1;
    while(pl)
    {
        i++;
        pl=pl->next;
    }
    return(i);
}


//
//list the players logged in
//
void pllist()
{
    struct player* pl;
    echof(0,"Current playerlist (number of players is %d):\n",numberOfPlayers());
    pl=player1;
    while(pl)
    {
        echof(0,"Name: %s\tScore: %s\tReady: %d\tTimeout: %d\tIP:%s port:%d socket:%d\n",pl->name,pl->score,pl->ready,pl->alive,pl->ip, pl->port, pl->foglalat);
        pl=pl->next;
    }
}


//
// find the player connected to a socket
//
struct player* foglalat2player(int fogl)
{
    struct player* pl;
    pl=player1;
    while(pl)
    {
        if(pl->foglalat==fogl) return(pl);
        pl=pl->next;
    }
    return(NULL);
}


void removePlayer(struct player* pl2rem)
{
    struct player* pl;
    //sendf(pl2rem->foglalat,"INFO:Goodbye, if you still hear me...");
    echof(0,"removing client %s socket %d port %d\n",pl2rem->ip,pl2rem->foglalat,pl2rem->port);

    if (pl2rem==player1) player1=pl2rem->next;
    else
    {
        pl=player1;
        while(pl)
        {
            if (pl->next==pl2rem) pl->next=pl2rem->next;
            pl=pl->next;
        }
    }
    shutdown(pl2rem->foglalat,SHUT_RDWR);
    close(pl2rem->foglalat);
    free(pl2rem);
    if (!player1)                //Last player has left
    {
        echof(0,"Last player has left, game is over.\n");
        gameIsOn=0;
    }
}


void shutDown(int signum)
// on exit close all connection
{
    struct player* pl;
    send2all("INFO:Server is shutting down.");
    pl=player1;
    while(pl)
    {
        removePlayer(pl);
        pl=pl->next;
    }
    close(foglalat);
    closeLogfile();
    exit(0);
}


//
// check for timed out clients
//
void aliveCheck()
{
    struct player *pl, *pl1;
    pl=player1;
    while(pl)
    {
        (pl->alive)++;
        if (pl->alive > TIMEOUT)
        {
            echof(0,"Player %d (%s) has timed out.\n", pl->id,pl->name);
            pl1=pl;
            pl=pl->next;
            removePlayer(pl1);
        } else
        {
            pl=pl->next;
        }
    }
}


void addPlayer(int foglalat)
{
    int uj;
    struct player* newPlayer;
    struct sockaddr_in ugyfel;
    socklen_t meret;
    meret = sizeof(ugyfel);
    uj = accept(foglalat, (struct sockaddr *) &ugyfel, &meret);
    if (uj<0)
    {
        perror("foglalat()");
        echof(0,"Error accepting a client!\n");
        return;
    }
    newPlayer=malloc(sizeof(struct player));
    if (!newPlayer) {
        echof(0,"malloc() failed while accepting client.\n");
        close(uj);
        return;
    }
    memset(newPlayer, 0, sizeof(*newPlayer));
    newPlayer->next=player1;
    player1=newPlayer;
    newPlayer->foglalat=uj;
    newPlayer->ready=0;
    newPlayer->alive=-1;
    newPlayer->p=0;
    newPlayer->id=-1;
    newPlayer->score=0;
    newPlayer->name[0]='\0';
    /* inet_ntoa returns a static buffer; copy it into our player struct safely. */
    snprintf(newPlayer->ip, sizeof(newPlayer->ip), "%s", inet_ntoa(ugyfel.sin_addr));
    newPlayer->port=ntohs(ugyfel.sin_port);
    if (numberOfPlayers() > MAX_PLAYERS)
    {
        sendf(uj,"REJECT:Sorry, no more players can be served!");
        removePlayer(newPlayer);
        return;
    }
    if (gameIsOn)
    {
        sendf(uj,"REJECT:Sorry, the game is on, please try a bit later!");
        removePlayer(newPlayer);
        return;
    }
    echof(0,"Accepted a new player from: %s port: %hd socket: %d\n",inet_ntoa(ugyfel.sin_addr),ntohs(ugyfel.sin_port),uj);
}


//add a new player, if there are not too many already..

int startNewGame()
{
    struct player* pl;
    struct node* nod;
    int id=0;
    int i=3;
    char shuffle[8];
    if (!player1)
    {
        echof(0,"Game is not started, as there are no players connected.\n");
        return(1);
    }
    newGame();                   // from engine.c
    // Send initial data to players
    send2all("START:Starting new game");
    // send IDs
    for(pl=player1;pl;pl=pl->next)
    {
        sendf(pl->foglalat,"%d\0",id);
        pl->id=id;
        pl->score=0;
        id++;
    }

    // send names
    for(pl=player1;pl;pl=pl->next)
    {
        debug(0,"send name: %s\n",pl->name);
        send2all("%s",pl->name);
    }
    send2all(".");

    // send words' length
    echof(1,"sending words' length to all\n");
    for(nod=head;nod;nod=nod->next)
    {
        echof(10,"sending length %d for word %s\n",nod->length,nod->anagram);
        send2all("%d",nod->length);
    }
    send2all(".");

    //send shuffled word
    nod=head;
    while(nod->next) nod=nod->next;
    strcpy(shuffle,nod->anagram);
    shuffleString(shuffle);
    echof(1,"sending shuffled word to all\n");
    send2all("%s\0",shuffle);

    //countdown
    echof(1,"counting down: %d\n",i);
    send2all(" ready",i); sleep(1);
    echof(1,"ready...");
    send2all(" steady",i); sleep(1);
    echof(1,"steady...");
    send2all(" go!!! ",i); sleep(1);
    echof(1,"go!!!\n");

    send2all(".");
    gameStart=time(0);
    gameIsOn=1;
    return(0);
}


void endGame()
{
    struct node* nod;
    struct player* current;
    current=player1;
    while(current)
    {
        current->ready=0;
        current=current->next;
    }
    gameIsOn=0;
    send2all("T:0");
    send2all("END");
    nod=head;
    while(nod)
    {
        send2all("%s",nod->anagram);
        nod=nod->next;
    }
    send2all(".");
}


//
// get a line from STDIN and send it to all
//
void mond()
{
    size_t hossz=255;
    char *msg;
    struct player* pl;
    int num;
    msg=malloc(hossz*sizeof(char));
    if (!msg) error(1,errno,"malloc()");

    ssize_t len = getline(&msg,&hossz, stdin);
    if (len <= 0) {
        free(msg);
        return;
    }

    if (len > 0 && (msg[len-1] == '\n' || msg[len-1] == '\r')) msg[len-1]='\0';
    if (len > 1 && msg[len-2] == '\r') msg[len-2]='\0';

    //message[strlen(message)]=DELIM;
    send2all("%s", msg);
    if (!strcmp(msg,"quit")) shutDown(0);
    if (!strcmp(msg,"list")) pllist();
    if (!strcmp(msg,"start")) startNewGame();
    if (!strcmp(msg,"solve")) endGame();
    if (!strncmp(msg,"time",4))
    {
        num=atoi(&(msg[5]));
        if (num>=0)
        {
            availableTime=num;
            echof(0,"Game time was changed to %d seconds.\n",availableTime);
        }
        else echof(0,"Game time cannot be set to %d seconds, it is still %d.\n",num,availableTime);
    }
    if (!strncmp(msg,"drop",4))
    {
        pl=foglalat2player(atoi(&(msg[5])));
        if (pl) removePlayer(pl);
    }
}


int playerReady(struct player* pl)
// a player becomes ready
{
    struct player* current;
    int go=1;
    if (pl->ready) return(1);    //he was already ready
    pl->ready=1;
    send2all("INFO:Player %d is ready.",pl->id);
    //start new game, if everyone is ready
    current=player1;
    while(current)
    {
        if (!current->ready) go=0;
        current=current->next;
    }
    if (go && numberOfPlayers() >= autoStart && autoStart) startNewGame();
    return(0);
}


void recv_msg(struct player* peer)
// message arrives from a player
{
    int n=1;
    char *msg=NULL;
    struct node* nod;
    if(peer->p>=BS) error(1,0,"Buffer is full\n");
    n=read(peer->foglalat,&peer->buff[peer->p],BS-(peer->p));
    if (n<0) perror("read()");
    debug(0,"Got message from socket %d:\n%s\n",peer->foglalat,&peer->buff[peer->p]);
    if (n==0)
    {
        echof(1,"Connection to player %d is closed\n",(int)peer->foglalat);
        //send2all("QUIT:%d:connection to %s is lost:",peer->id,peer->name);
        removePlayer(peer);
        return;
    }
    peer->alive=0;               //got message
    //for (r=0;r<n;r++) fprintf(stderr,"ch(%d)",peer->buff[(peer->p)+r]);
    //fflush(stderr);
    peer->p+=n;                  // ???
    // Parse messages per line
    while (getAline(&msg,peer->buff,&peer->p))
    {
        switch(msg[0])
        {
            case 'r':            //ready : Start new game if everyone is ready
                playerReady(peer);
                break;
            case 'q':            //quit
                removePlayer(peer);
                break;
            case 'n':            //name:xyxyxy
                strncpy(peer->name,&(msg[5]),8);
                // filter out ":"s !
                peer->name[8]='\0';
                echof(0,"Client on socket %d sets his/her name to %s\n",peer->foglalat,&msg[5]);
                break;
            case 'g':            //Guess from a player
                echof(0,"Player %d (%s) guesses the word \"%s\"\n",peer->id,peer->name,&msg[2]);
                if (!gameIsOn) return;
                nod=head;
                while(nod)
                {
                    if (strcmp(nod->anagram,&msg[2])==0)
                    {
                        if (!nod->guessed)
                        {
                            //if (nod->length==7) peer->score*=2;
                            //else
                            peer->score+=nod->length;
                            echof(0,"OK, player %d now has %d points.\n",peer->id,nod->length);
                            send2all("G:%d:%s:%d:%d",peer->id,nod->anagram,nod->id,peer->score);
                            nod->guessed=(peer->id)+1;
                            return;
                        }
                        else
                        {
                            echof(0,"The word \"%s\" already had been found by player %d.\n",&msg[2],(nod->guessed)-1);
                            sendf(peer->foglalat,"F:%s",&msg[2]);
                            return;
                        }
                    }
                    nod=nod->next;
                }
                //Guess failed
                echof(0,"No such word in my dictionary.\n");
                sendf(peer->foglalat,"F:%s",&msg[2]);
                return;
            case 't':            //heartbeat
                break;
            case 's':            //solve it (1 player mode only)
                echof(0,"Player %s gave up the game.\n",peer->name);
                if (numberOfPlayers()==1)
                {
                    echof(0,"No others left, so ending the game.\n");
                    endGame();
                }
                else
                {
                    echof(0,"There are players still playing (%d) ... continuing the game.",numberOfPlayers()-1);
                }

                break;
            default:
                echof(1,"Got an invalid message from player %d (%s)\n",peer->id,peer->name);
        }
        free(msg);

    }

}


void sigpipe_hndl(int sig)
{
    debug(0,"Broken pipe.\n");
    return;
}


int main(int argc, char *argv[])
{
    int port=PORT;
    int nbst=0;
    int r;
    fd_set rd;
    struct timeval tv;
    struct player* current;
    char * logfile="./nog_srv.log";
    char * wordlist="./wordlist.txt";
    debugLevel=10;

    GOptionContext *context;
    GError *error = NULL;
    static GOptionEntry entries[5];
    asprintf(&logfile,"./%s.log",PROGNAME);

    gchar * port_text;
    gchar * wordlist_text;
    gchar * logfile_text;
    gchar * debuglevel_text;

    asprintf(&port_text,"TCP port number (default: %d).",PORT);
    entries[0]=( GOptionEntry)
    {
        "port", 'p', 0, G_OPTION_ARG_INT, &port, port_text, "number"
    };

    asprintf(&wordlist_text,"File containing the list of valid words (default: %s).",wordlist);
    entries[1]=(  GOptionEntry)
    {
        "wordlist", 'w', 0, G_OPTION_ARG_FILENAME, &wordlist, wordlist_text, "file"
    } ;

    asprintf(&logfile_text,"Logfile (default: %s).",logfile);
    entries[2]=(  GOptionEntry)
    {
        "logfile", 'l', 0, G_OPTION_ARG_FILENAME, &logfile, logfile_text, "file"
    } ;

    asprintf(&debuglevel_text,"Debug level ( 0 - 10 ; 10 is the most verbose; default: %d).",debugLevel);
    entries[3]=(  GOptionEntry)
    {
        "debuglevel", 'd', 0, G_OPTION_ARG_INT, &debugLevel, debuglevel_text, "level"
    } ;

    entries[4]=( GOptionEntry ) {NULL};

    context = g_option_context_new ("\n\nServer for Net-o-Grama, a networked multiplayer anagram game.");

    g_option_context_add_main_entries(context, entries,PROGNAME);
    if (!g_option_context_parse(context, &argc, &argv,  &error))
    {
        g_warning("Error parsing command line options: %s",
            error->message);
        exit(EXIT_FAILURE);
    }
    g_option_context_free(context);

    free(port_text);free(wordlist_text);free(logfile_text);free(debuglevel_text);

    initLogfile(logfile);
    createDLBTree(&dlbHead, wordlist);
    head=NULL;
    srand(time(NULL));
    foglalat=_listen(port);
    signal(SIGTERM,shutDown);
    signal(SIGINT,shutDown);
    signal(SIGPIPE,sigpipe_hndl);
    while(!quit)
    {
        FD_ZERO (&rd);
        FD_SET(0,&rd);
        FD_SET (foglalat,&rd);
        nbst=foglalat;
        tv.tv_sec=1;
        tv.tv_usec=0;
        current=player1;
        while (current)
        {
            FD_SET(current->foglalat,&rd);
            if (current->foglalat > nbst) nbst=current->foglalat;
            current=current->next;
        }
        r = select(nbst + 1, &rd, NULL, NULL, &tv);
        if (r<0)
        {
            perror("select");
            exit(1);
        }
        if (FD_ISSET(foglalat,&rd)) addPlayer(foglalat);
        if (FD_ISSET(0,&rd)) mond();
        current=player1;
        while (current)
        {
            if (FD_ISSET(current->foglalat,&rd)) recv_msg(current);
            current=current->next;
        }
        //Manage time
        timeNow = time(0) ;
        if (timeNow!=gameTime)
        {
            gameTime = timeNow;
            aliveCheck();
            if ((gameTime < gameStart + availableTime) && gameIsOn)
            {
                send2all("T:%d",availableTime-(gameTime-gameStart));
            }
            else if (gameIsOn)   //Game is Over
            {
                endGame();
            }
        }
    }
    shutDown(0);

    return(0);
}
