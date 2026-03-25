#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <stdarg.h>
#include <signal.h>
#include "dlb.h"
#include "client.h"

extern int nop,id;
extern struct gamer gamers[MAX_PLAYERS];
extern int debugLevel;
int maxx,maxy;
int key_SHUFFLE=' ';
int key_CHECK='\n';
int key_CHECK_NORET=KEY_RIGHT;
int key_CLEAR=KEY_BACKSPACE;
int key_DELCHAR=KEY_LEFT;
int key_SOLVE='\t';
int key_QUIT=27;                 //escape

unsigned char charset[] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    126, 129, 165, 129, 189, 153, 129, 126,
    126, 255, 219, 255, 195, 231, 255, 126,
    108, 254, 254, 254, 124, 56, 16, 0,
    16, 56, 124, 254, 124, 56, 16, 0,
    56, 124, 56, 254, 254, 124, 56, 124,
    16, 16, 56, 124, 254, 124, 56, 124,
    0, 0, 24, 60, 60, 24, 0, 0,
    255, 255, 231, 195, 195, 231, 255, 255,
    0, 60, 102, 66, 66, 102, 60, 0,
    255, 195, 153, 189, 189, 153, 195, 255,
    15, 7, 15, 125, 204, 204, 204, 120,
    60, 102, 102, 102, 60, 24, 126, 24,
    63, 51, 63, 48, 48, 112, 240, 224,
    127, 99, 127, 99, 99, 103, 230, 192,
    153, 90, 60, 231, 231, 60, 90, 153,
    128, 224, 248, 254, 248, 224, 128, 0,
    2, 14, 62, 254, 62, 14, 2, 0,
    24, 60, 126, 24, 24, 126, 60, 24,
    102, 102, 102, 102, 102, 0, 102, 0,
    127, 219, 219, 123, 27, 27, 27, 0,
    62, 99, 56, 108, 108, 56, 204, 120,
    0, 0, 0, 0, 126, 126, 126, 0,
    24, 60, 126, 24, 126, 60, 24, 255,
    24, 60, 126, 24, 24, 24, 24, 0,
    24, 24, 24, 24, 126, 60, 24, 0,
    0, 24, 12, 254, 12, 24, 0, 0,
    0, 48, 96, 254, 96, 48, 0, 0,
    0, 0, 192, 192, 192, 254, 0, 0,
    0, 36, 102, 255, 102, 36, 0, 0,
    0, 24, 60, 126, 255, 255, 0, 0,
    0, 255, 255, 126, 60, 24, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    48, 120, 120, 48, 48, 0, 48, 0,
    108, 108, 108, 0, 0, 0, 0, 0,
    108, 108, 254, 108, 254, 108, 108, 0,
    48, 124, 192, 120, 12, 248, 48, 0,
    0, 198, 204, 24, 48, 102, 198, 0,
    56, 108, 56, 118, 220, 204, 118, 0,
    96, 96, 192, 0, 0, 0, 0, 0,
    24, 48, 96, 96, 96, 48, 24, 0,
    96, 48, 24, 24, 24, 48, 96, 0,
    0, 102, 60, 255, 60, 102, 0, 0,
    0, 48, 48, 252, 48, 48, 0, 0,
    0, 0, 0, 0, 0, 48, 48, 96,
    0, 0, 0, 252, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 48, 48, 0,
    6, 12, 24, 48, 96, 192, 128, 0,
    124, 198, 206, 222, 246, 230, 124, 0,
    48, 112, 48, 48, 48, 48, 252, 0,
    120, 204, 12, 56, 96, 204, 252, 0,
    120, 204, 12, 56, 12, 204, 120, 0,
    28, 60, 108, 204, 254, 12, 30, 0,
    252, 192, 248, 12, 12, 204, 120, 0,
    56, 96, 192, 248, 204, 204, 120, 0,
    252, 204, 12, 24, 48, 48, 48, 0,
    120, 204, 204, 120, 204, 204, 120, 0,
    120, 204, 204, 124, 12, 24, 112, 0,
    0, 48, 48, 0, 0, 48, 48, 0,
    0, 48, 48, 0, 0, 48, 48, 96,
    24, 48, 96, 192, 96, 48, 24, 0,
    0, 0, 252, 0, 0, 252, 0, 0,
    96, 48, 24, 12, 24, 48, 96, 0,
    120, 204, 12, 24, 48, 0, 48, 0,
    124, 198, 222, 222, 222, 192, 120, 0,
    48, 120, 204, 204, 252, 204, 204, 0,
    252, 102, 102, 124, 102, 102, 252, 0,
    60, 102, 192, 192, 192, 102, 60, 0,
    248, 108, 102, 102, 102, 108, 248, 0,
    254, 98, 104, 120, 104, 98, 254, 0,
    254, 98, 104, 120, 104, 96, 240, 0,
    60, 102, 192, 192, 206, 102, 62, 0,
    204, 204, 204, 252, 204, 204, 204, 0,
    120, 48, 48, 48, 48, 48, 120, 0,
    30, 12, 12, 12, 204, 204, 120, 0,
    230, 102, 108, 120, 108, 102, 230, 0,
    240, 96, 96, 96, 98, 102, 254, 0,
    198, 238, 254, 254, 214, 198, 198, 0,
    198, 230, 246, 222, 206, 198, 198, 0,
    56, 108, 198, 198, 198, 108, 56, 0,
    252, 102, 102, 124, 96, 96, 240, 0,
    120, 204, 204, 204, 220, 120, 28, 0,
    252, 102, 102, 124, 108, 102, 230, 0,
    120, 204, 224, 112, 28, 204, 120, 0,
    252, 180, 48, 48, 48, 48, 120, 0,
    204, 204, 204, 204, 204, 204, 252, 0,
    204, 204, 204, 204, 204, 120, 48, 0,
    198, 198, 198, 214, 254, 238, 198, 0,
    198, 198, 108, 56, 56, 108, 198, 0,
    204, 204, 204, 120, 48, 48, 120, 0,
    254, 198, 140, 24, 50, 102, 254, 0,
    120, 96, 96, 96, 96, 96, 120, 0,
    192, 96, 48, 24, 12, 6, 2, 0,
    120, 24, 24, 24, 24, 24, 120, 0,
    16, 56, 108, 198, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 255,
    48, 48, 24, 0, 0, 0, 0, 0,
    0, 0, 120, 12, 124, 204, 118, 0,
    224, 96, 96, 124, 102, 102, 220, 0,
    0, 0, 120, 204, 192, 204, 120, 0,
    28, 12, 12, 124, 204, 204, 118, 0,
    0, 0, 120, 204, 252, 192, 120, 0,
    56, 108, 96, 240, 96, 96, 240, 0,
    0, 0, 118, 204, 204, 124, 12, 248,
    224, 96, 108, 118, 102, 102, 230, 0,
    48, 0, 112, 48, 48, 48, 120, 0,
    12, 0, 12, 12, 12, 204, 204, 120,
    224, 96, 102, 108, 120, 108, 230, 0,
    112, 48, 48, 48, 48, 48, 120, 0,
    0, 0, 204, 254, 254, 214, 198, 0,
    0, 0, 248, 204, 204, 204, 204, 0,
    0, 0, 120, 204, 204, 204, 120, 0,
    0, 0, 220, 102, 102, 124, 96, 240,
    0, 0, 118, 204, 204, 124, 12, 30,
    0, 0, 220, 118, 102, 96, 240, 0,
    0, 0, 124, 192, 120, 12, 248, 0,
    16, 48, 124, 48, 48, 52, 24, 0,
    0, 0, 204, 204, 204, 204, 118, 0,
    0, 0, 204, 204, 204, 120, 48, 0,
    0, 0, 198, 214, 254, 254, 108, 0,
    0, 0, 198, 108, 56, 108, 198, 0,
    0, 0, 204, 204, 204, 124, 12, 248,
    0, 0, 252, 152, 48, 100, 252, 0,
    28, 48, 48, 224, 48, 48, 28, 0,
    24, 24, 24, 0, 24, 24, 24, 0,
    224, 48, 48, 28, 48, 48, 224, 0,
    118, 220, 0, 0, 0, 0, 0, 0,
    0, 16, 56, 108, 198, 198, 254, 0,
    120, 204, 192, 204, 120, 24, 12, 120,
    0, 204, 0, 204, 204, 204, 126, 0,
    28, 0, 120, 204, 252, 192, 120, 0,
    126, 195, 60, 6, 62, 102, 63, 0,
    204, 0, 120, 12, 124, 204, 126, 0,
    224, 0, 120, 12, 124, 204, 126, 0,
    48, 48, 120, 12, 124, 204, 126, 0,
    0, 0, 120, 192, 192, 120, 12, 56,
    126, 195, 60, 102, 126, 96, 60, 0,
    204, 0, 120, 204, 252, 192, 120, 0,
    224, 0, 120, 204, 252, 192, 120, 0,
    204, 0, 112, 48, 48, 48, 120, 0,
    124, 198, 56, 24, 24, 24, 60, 0,
    224, 0, 112, 48, 48, 48, 120, 0,
    198, 56, 108, 198, 254, 198, 198, 0,
    48, 48, 0, 120, 204, 252, 204, 0,
    28, 0, 252, 96, 120, 96, 252, 0,
    0, 0, 127, 12, 127, 204, 127, 0,
    62, 108, 204, 254, 204, 204, 206, 0,
    120, 204, 0, 120, 204, 204, 120, 0,
    0, 204, 0, 120, 204, 204, 120, 0,
    0, 224, 0, 120, 204, 204, 120, 0,
    120, 204, 0, 204, 204, 204, 126, 0,
    0, 224, 0, 204, 204, 204, 126, 0,
    0, 204, 0, 204, 204, 124, 12, 248,
    195, 24, 60, 102, 102, 60, 24, 0,
    204, 0, 204, 204, 204, 204, 120, 0,
    24, 24, 126, 192, 192, 126, 24, 24,
    56, 108, 100, 240, 96, 230, 252, 0,
    204, 204, 120, 252, 48, 252, 48, 48,
    248, 204, 204, 250, 198, 207, 198, 199,
    14, 27, 24, 60, 24, 24, 216, 112,
    28, 0, 120, 12, 124, 204, 126, 0,
    56, 0, 112, 48, 48, 48, 120, 0,
    0, 28, 0, 120, 204, 204, 120, 0,
    0, 28, 0, 204, 204, 204, 126, 0,
    0, 248, 0, 248, 204, 204, 204, 0,
    252, 0, 204, 236, 252, 220, 204, 0,
    60, 108, 108, 62, 0, 126, 0, 0,
    56, 108, 108, 56, 0, 124, 0, 0,
    48, 0, 48, 96, 192, 204, 120, 0,
    0, 0, 0, 252, 192, 192, 0, 0,
    0, 0, 0, 252, 12, 12, 0, 0,
    195, 198, 204, 222, 51, 102, 204, 15,
    195, 198, 204, 219, 55, 111, 207, 3,
    24, 24, 0, 24, 24, 24, 24, 0,
    0, 51, 102, 204, 102, 51, 0, 0,
    0, 204, 102, 51, 102, 204, 0, 0,
    34, 136, 34, 136, 34, 136, 34, 136,
    85, 170, 85, 170, 85, 170, 85, 170,
    219, 119, 219, 238, 219, 119, 219, 238,
    24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 248, 24, 24, 24,
    24, 24, 248, 24, 248, 24, 24, 24,
    54, 54, 54, 54, 246, 54, 54, 54,
    0, 0, 0, 0, 254, 54, 54, 54,
    0, 0, 248, 24, 248, 24, 24, 24,
    54, 54, 246, 6, 246, 54, 54, 54,
    54, 54, 54, 54, 54, 54, 54, 54,
    0, 0, 254, 6, 246, 54, 54, 54,
    54, 54, 246, 6, 254, 0, 0, 0,
    54, 54, 54, 54, 254, 0, 0, 0,
    24, 24, 248, 24, 248, 0, 0, 0,
    0, 0, 0, 0, 248, 24, 24, 24,
    24, 24, 24, 24, 31, 0, 0, 0,
    24, 24, 24, 24, 255, 0, 0, 0,
    0, 0, 0, 0, 255, 24, 24, 24,
    24, 24, 24, 24, 31, 24, 24, 24,
    0, 0, 0, 0, 255, 0, 0, 0,
    24, 24, 24, 24, 255, 24, 24, 24,
    24, 24, 31, 24, 31, 24, 24, 24,
    54, 54, 54, 54, 55, 54, 54, 54,
    54, 54, 55, 48, 63, 0, 0, 0,
    0, 0, 63, 48, 55, 54, 54, 54,
    54, 54, 247, 0, 255, 0, 0, 0,
    0, 0, 255, 0, 247, 54, 54, 54,
    54, 54, 55, 48, 55, 54, 54, 54,
    0, 0, 255, 0, 255, 0, 0, 0,
    54, 54, 247, 0, 247, 54, 54, 54,
    24, 24, 255, 0, 255, 0, 0, 0,
    54, 54, 54, 54, 255, 0, 0, 0,
    0, 0, 255, 0, 255, 24, 24, 24,
    0, 0, 0, 0, 255, 54, 54, 54,
    54, 54, 54, 54, 63, 0, 0, 0,
    24, 24, 31, 24, 31, 0, 0, 0,
    0, 0, 31, 24, 31, 24, 24, 24,
    0, 0, 0, 0, 63, 54, 54, 54,
    54, 54, 54, 54, 255, 54, 54, 54,
    24, 24, 255, 24, 255, 24, 24, 24,
    24, 24, 24, 24, 248, 0, 0, 0,
    0, 0, 0, 0, 31, 24, 24, 24,
    255, 255, 255, 255, 255, 255, 255, 255,
    0, 0, 0, 0, 255, 255, 255, 255,
    240, 240, 240, 240, 240, 240, 240, 240,
    15, 15, 15, 15, 15, 15, 15, 15,
    255, 255, 255, 255, 0, 0, 0, 0,
    0, 0, 118, 220, 200, 220, 118, 0,
    0, 120, 204, 248, 204, 248, 192, 192,
    0, 252, 204, 192, 192, 192, 192, 0,
    0, 254, 108, 108, 108, 108, 108, 0,
    252, 204, 96, 48, 96, 204, 252, 0,
    0, 0, 126, 216, 216, 216, 112, 0,
    0, 102, 102, 102, 102, 124, 96, 192,
    0, 118, 220, 24, 24, 24, 24, 0,
    252, 48, 120, 204, 204, 120, 48, 252,
    56, 108, 198, 254, 198, 108, 56, 0,
    56, 108, 198, 198, 108, 108, 238, 0,
    28, 48, 24, 124, 204, 204, 120, 0,
    0, 0, 126, 219, 219, 126, 0, 0,
    6, 12, 126, 219, 219, 126, 96, 192,
    56, 96, 192, 248, 192, 96, 56, 0,
    120, 204, 204, 204, 204, 204, 204, 0,
    0, 252, 0, 252, 0, 252, 0, 0,
    48, 48, 252, 48, 48, 0, 252, 0,
    96, 48, 24, 48, 96, 0, 252, 0,
    24, 48, 96, 48, 24, 0, 252, 0,
    14, 27, 27, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 216, 216, 112,
    48, 48, 0, 252, 0, 48, 48, 0,
    0, 118, 220, 0, 118, 220, 0, 0,
    56, 108, 108, 56, 0, 0, 0, 0,
    0, 0, 0, 24, 24, 0, 0, 0,
    0, 0, 0, 0, 24, 0, 0, 0,
    15, 12, 12, 12, 236, 108, 60, 28
};
static WINDOW *playField, *scoreBoard, *guessBoard;
static int useColors=0;
extern struct node* head;

void outline(WINDOW* win,char shadow, char outbyte)
{
    int bc;
    int y,x;
    #define SPACE waddch(win,' '|COLOR_PAIR(16)|A_REVERSE)
    getyx(win,y,x);
    if (shadow) SPACE; else wmove(win,y,x+2);
    for (bc = 7; bc >= 0; bc--)
    {
        if (outbyte & (0x01 << bc))
        {
            if (shadow) waddch(win,' '|COLOR_PAIR(17));
            else waddch(win,' '|COLOR_PAIR(15)|A_REVERSE);
        }
        else
        {
            if (shadow) waddch(win,' '|COLOR_PAIR(15));
            else
            {
                getyx(win,y,x);
                wmove(win,y,x+1);
                if (x>68) wmove(win,y+1,0);
            }
        }
    }
    if (shadow) SPACE;
}


void blWord(WINDOW* win,char *str,char shadow)
{
    unsigned char ch;
    int ch_off;
    unsigned char *ch_addr;
    unsigned char *font;
    int linenum,chnum;
    char outstr[7]="       ";
    strncpy(outstr, str, strlen(str));
    font=(unsigned char *)&charset;
    for (linenum = 0; linenum < 8; linenum++)
    {
        for (chnum = 0; chnum < 7; chnum++)
        {
            ch      = outstr[chnum]-(32*(outstr[chnum]>=97 && outstr[chnum]<=122));
            ch_off  = (int) ch * 8;
            ch_addr = font + ch_off + linenum;
            outline(win, shadow, *ch_addr);
        }
    }
}


void updatePlayField(char *Word, char *Try, int new)
{
    wclear(playField);
    wattrset(playField,COLOR_PAIR(16)|A_REVERSE);
    wprintw(playField,"                                                                      ");
    wattrset(playField,COLOR_PAIR(15));
    blWord(playField,Word,1);
    blWord(playField,Try,1);
    wmove(playField,1,0);
    blWord(playField,Word,0);
    blWord(playField,Try,0);
    wrefresh(playField);
}


void initPlayField(char *Word)
{
    updatePlayField(Word,"       ",1);
}


void displayMessage(char *Line1, char* Line2)
{
    updatePlayField(Line1, Line2,1);
}


void updateTime(int time)
{
    if (useColors) wcolor_set(scoreBoard,10,NULL);
    wattron(scoreBoard,A_BOLD);
    mvwprintw(scoreBoard,1,1,"TIME:%3d",time);
    //mvwprintw(scoreBoard,2,4,"%d",gameTime);
    //wcolor_set(scoreBoard,16,NULL);
    wattroff(scoreBoard,A_BOLD);
    box(scoreBoard,0,0);
    wrefresh(scoreBoard);
}


void drawScoreBoard(int refr)
{
    int i;
    if (useColors) wbkgd(scoreBoard,COLOR_PAIR(10));
    wmove(scoreBoard,3,0);
    for (i=0;i<nop;i++)
    {
        if (useColors) wcolor_set(scoreBoard,i+11,NULL);
        wattron(scoreBoard,A_BOLD);
        wprintw(scoreBoard,"%s\n    %d\n\n",gamers[i].name,gamers[i].score);
    }
    //if (useColors) wcolor_set(scoreBoard,8,NULL);
    //mvwvline(scoreBoard,0,0,0,16);
    updateTime(refr);
}


void drawGuessBoard(struct node* head, int refr)
{
    int i=0;
    int j=1;
    char line[]="----------";
    struct node* nod;
    wclear(guessBoard);
    //whline(guessBoard,0,80);
    //mvwaddch(guessBoard,0,70,ACS_BTEE);
    nod=head;
    while(nod)
    {
        wmove(guessBoard,i%7,j);
        if((nod->guessed) >0)
        {
            //if (nod->guessed)
            if (useColors) wcolor_set(guessBoard,nod->guessed,NULL);
            echof(0,"%s",nod->anagram);
            waddstr(guessBoard,nod->anagram);
            //else waddstr(guessBoard,str2upper(nod->anagram));
        }
        else
        {
            if (useColors) wcolor_set(guessBoard,8,NULL);
            //debug(0,"%s",nod->anagram);
            strncpy(line,"--------",(size_t) nod->length);
            line[strlen(nod->anagram)]='\0';
            waddstr(guessBoard,line);
        }
        if ( ++i%7 == 0 ) j+=(nod->length)+2;
        nod=nod->next;
    }
    if (useColors) wcolor_set(guessBoard,8,NULL);
    if (refr) wrefresh(guessBoard);
    wmove(guessBoard,0,0);
}


void sigScreen(int sig)
{
    clear();
    endwin();
    signal(sig, SIG_DFL);
    if (sig) raise(sig);
}


void endScreen()
{
    clear();
    endwin();
}


void initScreen()
{
    initscr();
    raw();
    keypad(stdscr,TRUE);
    if (has_colors())
        // set up color pairs
    {
        useColors=1;
        start_color();
        standout();
        init_pair(1,1,0);
        init_pair(2,2,0);
        init_pair(3,COLOR_YELLOW,0);
        init_pair(4,COLOR_CYAN,0);
        init_pair(5,COLOR_BLUE,0);
        init_pair(8,COLOR_WHITE,0);
        init_pair(9,COLOR_MAGENTA,COLOR_CYAN);
        init_pair(10,COLOR_WHITE,COLOR_BLUE);
        init_pair(11,1,COLOR_BLUE);
        init_pair(12,2,COLOR_BLUE);
        init_pair(13,COLOR_YELLOW,COLOR_BLUE);
        init_pair(14,COLOR_CYAN,COLOR_BLUE);
        init_pair(15,COLOR_WHITE,COLOR_BLUE);
        init_pair(16,COLOR_BLUE,COLOR_WHITE);
        init_pair(17,COLOR_WHITE,COLOR_BLACK);

    }
    signal( SIGINT, sigScreen);
    signal( SIGTERM, sigScreen);
    atexit(endScreen);
    playField=newwin(17,70,0,0);
    scoreBoard=newwin(17,10,0,70);
    guessBoard=newwin(7,80,17,0);
    //wattron(playField,A_BOLD);
    wattron(scoreBoard,A_BOLD);
    wattron(guessBoard,A_BOLD);
    halfdelay(2);
    noecho();
    curs_set(0);
    getmaxyx(stdscr,maxy,maxx);
    if (maxy<24 || maxx <80)
    {
        endwin();
        fprintf(stderr,"This program requires a terminal with the minimum size of 80x24,\nwhile this terminal is only a %dx%d one.",maxy,maxx);
        exit(1);
    }

}


void gameLoop()
{
    fd_set rd;
    struct timeval tv;
    int r,key;
    while(1)
    {
        tv.tv_sec=1;
        tv.tv_usec=0;
        debug(9,"fd_zero\n");
        FD_ZERO(&rd);
        FD_SET(0,&rd);
        FD_SET(srv,&rd);
        debug(9,"select\n");
        r = select(srv + 1, &rd, NULL, NULL,&tv);
        if (r<0) error(1,0,"select()");
        if (FD_ISSET(srv,&rd)) recv_msg();
        if (FD_ISSET(0,&rd))
        {
            key=getch();
            keyPressed(key);
        }
        heartBeat();
    }
}
