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
#include "debug.h"
#include "linked.h"
#include "network.h"
#include "sound.h"
#include "engine.h"

extern int key_SHUFFLE;
extern int key_CHECK;
extern int key_CHECK_NORET;
extern int key_DELCHAR;
extern int key_SOLVE;
extern int key_QUIT;
extern int key_CLEAR;

extern void displayMessage(char *Line1, char *Line2);
extern void updatePlayField(char *Word, char *Try, int new);
extern void initPlayField(char *Word);
extern void drawScoreBoard(int refr);
extern void drawGuessBoard(struct node* head, int refr);
extern void updateTime(int refr);
struct gamer
{
    char name[9];
    int score;
    int state;
};

struct gamer gamers[MAX_PLAYERS];

struct node* head=NULL;
char rootWord[10];
char shuffle[]  = "        ";    //the actual shuffle of rootWord
char answer[]   = "         ";
char rem[]   = "         ";      //the actual remaining letters of shuffle
int blank[10];
int ans_len;
//int srv;
int quit=0;
int nop=0;                       // number of players
int delAnswer=1;                 // makes difference betwen pressing Enter  and Right_arrow.
extern int gameTime;
int gameIsOn=0;
int id;
char state=0;
int argc;
char **argv;
struct node* nod;
struct node* nod1;

// message buffer and its pointer
char buff[BS];
int p=0;
int nodeid;

int port=PORT;
char usrname[9];
char srvname[50]="localhost";

void clearAnswer()
{
    state=GAMEISON;
    if (!delAnswer) return;
    answer[0]='\0';
    ans_len=0;
    strncpy(rem,shuffle,sizeof(shuffle)-1);
    updatePlayField(rem,answer,0);
}


void keyPressed(int key)
{
    if (key==key_QUIT)
    {
        sendf(srv,"quit");
        exit(0);
    }

    switch(state)
    {

        case BEFORE:
            if (key==key_CHECK )
            {
                sendf(srv,"ready");
            }
            break;

        case GAMEISON:
            if (key==key_SHUFFLE)
            {
                shuffleString(rem);
                debug(2,"displaying %s and %s",rem,answer);
                updatePlayField(rem,answer,0);
                debug(1,"xxx");
            }
            if (key==key_CHECK_NORET || key==key_CHECK)
            {
                delAnswer=(key==key_CHECK);
                if (!strlen(answer)) return;
                sendf(srv,"g:%s",answer);
                state=WAITING;
            }
            if (key==key_CLEAR)  // BackSpace
            {
                delAnswer=1;
                clearAnswer();
            }
            if (key==key_DELCHAR)
            {
                if (ans_len<=0) break;
                rem[blank[ans_len]]=answer[ans_len-1];
                answer[--ans_len]='\0';
                updatePlayField(rem,answer,0);
            }
            if (key==key_SOLVE)
            {
                sendf(srv,"solve");
            }
            if (key>='a' && key<='z')
            {
                int pos;
                pos=whereinstr(rem,key);
                if (pos != -1)
                {
                    answer[ans_len++]=key;
                    answer[ans_len]='\0';
                    rem[pos]=' ';
                    blank[ans_len]=pos;
                    updatePlayField(rem,answer,0);
                }
            }
            if (key>='1' && key<='7')
            {
                int pos;
                pos=key-49;
                if (rem[pos]!=' ')
                {
                    answer[ans_len++]=rem[pos];
                    answer[ans_len]='\0';
                    rem[pos]=' ';
                    blank[ans_len]=pos;
                    updatePlayField(rem,answer,0);
                }
            }
            break;

        default:                 // don't handle keys in other states
            return;
    }
}


void clearList()
{
    int j;
    debug(8,"Clearing player list. max_players=%d\n",MAX_PLAYERS);
    for(j=0;j<MAX_PLAYERS;j++)
    {
        debug(9,"Clearing player %d\n",j);
        gamers[j].state=0;
        gamers[j].score=0;
        //memcpy(gamers[j].name,"        ",9);
    }
    debug(8,"Player list is cleared.\n");

}


struct node *prevNode=NULL;

void recv_msg()
{
    int n,pos,gid;
    char * msg=NULL;
    char tmpstr[10];             //max wordlength!!
    char word[]="          ";
    n=read(srv,&buff[p],BS-p);
    if (n<0) debug(0," error in read(): %d",errno);
    if (n==0)                    //Connection lost
    {
        debug(0,"Connection to server is lost, exiting now.\n");
        exit(1);
    }
    debug(2,"recevied %d bytes: %s\n",n,buff);
    p+=n;
    while (getAline(&msg,buff,&p))
    {
        switch(state)
        {
            case BEFORE:         //Before Game
                switch(msg[0])
                {
                    case 'R':    //reject
                        exit(1);
                        break;
                    case 'S' :   //start game
                        debug(0,"Starting  new game...\n");
                        debug(1,"Destroying previous answers\n");
                        destroyAnswers(&head);
                        nodeid=0;
                        head=NULL;
                        clearList();
                        nop=0;
                        strcpy(answer,"         ");
                        ans_len=0;
                        state=GAMEISON;
                        debug(0,"Waiting for the ID..\n");
                        state=GETID;
                        break;
                    default :
                        debug(1,"Message unknown: \"%s\" (first char (%d))\n", msg,(int) msg[0]);
                }
                break;

            case GETID:          //get id
                id=atoi(msg);
                state=GETNAMES;
                debug(0,"Got the id : %d\nWaiting for the names..",id);
                break;

            case GETNAMES:
                if(msg[0]=='.')  //player names
                {
                    debug(0,"Got last name, %d players alltogether.\n",nop);
                    nodeid=0;
                    prevNode=NULL;
                    state=GETNODES;
                }
                else
                {
                    gamers[nop].state=1;
                    gamers[nop].score=0;
                    strncpy(gamers[nop].name,msg,8);
                    debug(0,"Got the name of the next player: %s\n",gamers[nop].name);
                    nop++;
                }
                break;

            case GETNODES:
                if(msg[0]=='.')
                {
                    state=GETSHUFFLE;
                }
                else
                {
                    struct node* newNode=malloc(sizeof(struct node));
                    if (!newNode) error(1,errno,"malloc() for newNode");
                    if(!head) head=newNode;
                    debug(5,"Got node length %d to node id %d\n",atoi(msg),nodeid);
                    newNode->length=atoi(msg);
                    strncpy(newNode->anagram,"XXXXXXXXXX",newNode->length);
                    newNode->id=nodeid++;
                    newNode->anagram[newNode->length]='\0';
                    newNode->found = 0;
                    newNode->guessed = 0;
                    newNode->next = NULL;
                    if(prevNode) prevNode->next=newNode;
                    prevNode=newNode;
                }
                break;

            case GETSHUFFLE:
                debug(5,"Got shuffled word %s\n",msg);
                strncpy(shuffle,msg,8);
                shuffle[7]='\0';
                strcpy(rem,shuffle);
                strcpy(answer,"       ");
                state=COUNTDOWN;
                break;

            case COUNTDOWN:
                if (msg[0]=='.')
                {
                    gameTime=AVAILABLE_TIME;
                    state=GAMEISON;
                    //debug(0,"rootWord: %s",shuffle);
                    drawScoreBoard(0);
                    updateTime(gameTime);
                    initPlayField(rem);
                    drawGuessBoard(head,1);

                    //debug(0,"rootWord: %s",rootWord);
                    return;
                }
                strcpy(word,msg);
                displayMessage(word,"      ");
                Mix_PlayChannel(-1, getSound("clock-tick"), 0);
                break;

            case GAMEISON:
            case WAITING:
                switch(msg[0])
                {
                    case 'G':    //Someone guessed a word G:[id]:[word]:[nodeid]:[score]
                        pos=2;
                        nextWord(tmpstr,msg,&pos);
                        gid=atoi(tmpstr);
                        debug(5,"gid: %d\n",gid);
                        nextWord(word,msg,&pos);
                        word[strlen(word)]='\0';
                        debug(5,"word: %s",tmpstr);
                        nextWord(tmpstr,msg,&pos);
                        debug(5,"nodeid: %s",tmpstr);
                        nodeid=atoi(tmpstr);
                        nextWord(tmpstr,msg,&pos);
                        debug(5,"score: %s",tmpstr);
                        gamers[gid].score=atoi(tmpstr);
                        debug(0,"new score: %d",gamers[gid].score);
                        if (gid==id)//it was ME.. make some music here...
                        {
                            clearAnswer();
                            Mix_PlayChannel(-1, getSound("found"), 0);
                            delAnswer=1;
                        }
                        else
                        {
                            Mix_PlayChannel(-1, getSound("found2"), 0);
                        }
                        drawScoreBoard(1);
                        nod=head;
                        while(nod)
                        {
                            //debug(0,"%s\t%s\t%d\n",nod->anagram,word,strcmp(nod->anagram,word));
                            if (nod->id==nodeid)
                            {
                                nod->guessed=gid+1;
                                strcpy(nod->anagram,word);
                            }
                            nod=nod->next;
                        }
                        drawGuessBoard(head,1);
                        break;
                    case 'F':    //my last guess has failed
                        Mix_PlayChannel(-1, getSound("badword"), 0);
                        clearAnswer();
                        break;
                    case 'T' :   //set time
                        pos=2;
                        nextWord(tmpstr,msg,&pos);
                        gameTime=atoi(tmpstr);
                        updateTime(gameTime);
                        if (gameTime<=10) { Mix_PlayChannel(-1, getSound("clock-tick"), 0);}
                        break;
                    case 'E' :   //game ended
                        state=ENDED;
                        nod1=head;
                        break;
                }
                break;
            case ENDED:
                if(msg[0]=='.')
                {
                    displayMessage(" game","  over");
                    drawScoreBoard(1);
                    drawGuessBoard(head,1);
                    state=BEFORE;
                }
                else
                {
                    if(!nod1) { debug(0,"Too many words received!"); }
                    else
                    {
                        strncpy(word,msg,8);
                        word[strlen(word)]='\0';
                        strcpy(nod1->anagram,word);
                                 //found by the server
                        if (!nod1->guessed) nod1->guessed=MAX_PLAYERS+1;
                        nod1=nod1->next;
                    }
                }
                break;
        }
    }
    if (msg) free(msg);
    return;
}


void initConnection(char* server, int port)
{
    srv=connect2server(server,port);
    sendf(srv,"name: %s",usrname);
}


void heartBeat()
{
    sendf(srv,"t");
}
