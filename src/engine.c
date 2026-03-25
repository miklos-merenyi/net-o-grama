#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <curses.h>
#include <getopt.h>
#include <sys/stat.h>
#include "debug.h"
#include "dlb.h"
#include "linked.h"
#include "network.h"
#define SPACE_CHAR ' '

//module level variables for game control
extern struct node* head;
struct dlb_node* dlbHead;
extern char rootWord[10];
char spc=' '+32;
int blank[9];
int lettersNum = 7;
int gameStart = 0;
int gameTime=AVAILABLE_TIME;
int stopTheClock = 0;
int totalScore = 0;
int score = 0;
int answersSought = 0;
int answersGot = 0;
int bigWordLen = 0;
int updateTheScore = 0;
int gamePaused = 0;
int foundDuplicate = 0;
int quitGame = 0;
int winGame = 0;
int netGame = 0;

void swap(struct node** from, struct node** to)
{
    // swaps the contents of 2 linked list nodes
    // doesn't disturb the pointers

    char* swap;

    swap = malloc(sizeof((*from)->anagram));
    strcpy(swap, (*from)->anagram);

    strcpy((*from)->anagram, (*to)->anagram);
    (*from)->length = (*to)->length;
    strcpy((*to)->anagram, swap);
    (*to)->length = strlen(swap);

}


void sort(struct node** headRef)
{
    // sort the linked list alpha/num of chars
    struct node* current = *headRef;
    struct node* next = malloc(sizeof(struct node));
    int done = 0;
    int swaps = 0;

    // walk the list
    while (!done)
    {
        while (current !=NULL)
        {
            next = current->next;
            if (next != NULL)
            {
                //printf("%s, %s - %i\n", next->anagram, current->anagram, strcmp(next->anagram, current->anagram));
                if (strcmp(next->anagram, current->anagram)<0)
                {
                    swap(&next, &current);
                    swaps++;
                }
            }
            current = current->next;
        }
        if (!swaps)
        {
            done = 1;
        }
        else
        {
            swaps = 0;
            current = *headRef;
        }
    }

    done = 0;
    current = *headRef;
    swaps = 0;

    // walk the list
    while (!done)
    {
        while (current !=NULL)
        {
            next = current->next;
            if (next != NULL)
            {
                //printf("%s, %s \n", next->anagram, current->anagram);
                if (strlen(next->anagram) < strlen (current->anagram))
                {
                    swap(&next, &current);
                    swaps++;
                }
            }
            current = current->next;
        }
        if (!swaps)
        {
            done = 1;
        }
        else
        {
            swaps = 0;
            current = *headRef;
        }
    }
    free(next);

    // generate ids
    done = 0;
    current = *headRef;

    while(current)
    {
        current->id=done++;
        current=current->next;
    }

}


void push(struct node** headRef, char* anagram)
{

    struct node* newNode;
    int len;
    struct node* current = *headRef;

    newNode = malloc(sizeof(struct node));
    // walk the list first, so we can ignore duplicates...
    // this is probably slower than clearing duplicates at the end
    // but simpler to write in the first instance
    while (current != NULL)
    {
        if (strcmp(anagram, current->anagram)==0)
        {
            return;
        }
        current = current->next;
    }
    len = strlen(anagram);
    //debug("ADDED %s\t LENGTH: %d", anagram,len);
    //newNode->anagram = malloc(sizeof(char)*len+1);
    strcpy(newNode->anagram, anagram);
    newNode->anagram[len]='\0';
    newNode->length = len;
    newNode->found = 0;
    newNode->guessed = 0;
    newNode->next = *headRef;    // dereference back the the real head pointer
    *headRef = newNode;          // ditto when replacing it with the new one
}


char* shiftLeftKill(char* string)
{
    // shift a string of characters 1 character to the left

    int i;
    char newString[50];
    int len;

    len = strlen(string);
    //newString = malloc(sizeof(char) * (len));
    for (i=1;i<len;i++)
    {
        newString[i-1] = string[i];
    }
    newString[len-1] = '\0';
    return(strdup(newString));
}


/***********************************************************/
char* shiftLeft(char* string)
{
    // shift a string of characters 1 character to the left
    // move the first character to the end of the string

    int i;
    char start;
    char newString[50];
    int len;

    len = strlen(string);

    //newString = malloc(sizeof(char) * (len+1));

    start = string[0];

    for (i=1;i<len;i++)
    {
        newString[i-1] = string[i];
    }

    newString[len-1] = start;
    newString[len] = '\0';

    return(strdup(newString));
    //free(newString);
}


char* ag(char* guess, char* remain)
{
    // generate all possible combinations of the root word
    // the initial letter is fixed (hence the space character
    // at the end of the possible list)

    char  newGuess[10];
    char  newRemain[10];
    int    totalLen=0, guessLen=0, remainLen=0, i;

    // allocate space for our working variables
    guessLen = strlen(guess);
    remainLen = strlen(remain);
    totalLen = guessLen + remainLen;

    //newGuess = malloc(sizeof(char) * (totalLen+1));
    //newRemain = malloc(sizeof(char) * (totalLen+1));

    // move last remaining letter to end of guess
    strcpy(newGuess, guess);
    strcpy(newRemain, remain);
    newGuess[guessLen] = newRemain[remainLen-1];
    newGuess[guessLen+1] = '\0';
    newRemain[remainLen-1] = '\0';

    //debug(0,"%s\n", newGuess);

    if(strlen(newGuess) > 3)
    {
        if (dlb_lookup(dlbHead,shiftLeftKill(newGuess)))
        {
            push(&head, shiftLeftKill(newGuess));
        }
    }

    if (strlen(newRemain))
    {
        ag(newGuess, newRemain);

        for (i=totalLen-1;i>0;i--)
        {
            if(strlen(newRemain) > i)
            {
                strcpy(newRemain, shiftLeft(newRemain));
                ag(newGuess, newRemain);
            }
        }
    }
    return rootWord;
    // free the space
    //free(newGuess);
    //free(newRemain);
}


char* swapChars(int from, int to, char* string)
{
    // swap 2 characters in a string
    debug(10,"swapping %d and %d in %s",from,to, string);
    char swap;
    swap = string[from];
    string[from] = string[to];
    string[to] = swap;
    return string;
}


void shuffleString(char* thisWord)
{
    // replace characters randomly
    int numSwaps,from,to,i,len;
    debug(8, "Shuffling string: %s",thisWord);
    len=strlen(thisWord);
    numSwaps = (rand()%len)+20;
    for (i=0;i<numSwaps;i++)
    {
        from = rand()%len;
        to = rand()%len;
        if ( ((thisWord)[from] != spc) & ((thisWord)[to] != spc))
        {
            strcpy(thisWord, swapChars(from, to, thisWord));
        }
    }
}


char* getRandomWord()
{
    FILE* wordlist;
    int filelocation;
    int i;
    char* wordFromList = malloc(sizeof(char) * 50);
    int len;
    int done = 0;
    filelocation = rand()%1000;
    if ((wordlist=fopen("wordlist.txt","r"))==NULL )
    {
        error(1,errno,"Can't open wordlist file");
    }

    for (i=0;i<=filelocation;i++)
    {
        if(fscanf(wordlist, "%s", wordFromList) != EOF)
        {
            // spin on
        }
        else
        {
            // go back to the start of the file
            fclose(wordlist);
            fopen("wordlist.txt", "r");
        }
    }

    // ok random location reached
    while (!done)
    {
        len = strlen(wordFromList);
        if ((len==lettersNum))
        {
            done = 1;
        }
        else
        {

            if(fscanf(wordlist, "%s", wordFromList) != EOF)
            {
                // spin on
            }
            else
            {
                // go back to the start of the file
                fclose(wordlist);
                fopen("wordlist.txt", "r");
                fscanf(wordlist, "%s", wordFromList);
            }
        }
    }

    fclose(wordlist);

    // add in our space character
    wordFromList[len] = ' ';
    wordFromList[len+1] = '\0';

    return wordFromList;
    free(wordFromList);
}


int nextBlank(char* string)
{
    // determine the next blank space in a string - blanks are indicated by pound   not space

    int i;
    int found=0;

    for(i=0;i<lettersNum;i++)
    {
        if (string[i]==SPACE_CHAR)
        {
            found = 1;
            break;
        }
    }
    if (found)
    {
        return i+1;
    }
    else
    {
        return 0;
    }
}


//
// Check, if answers fit the scrren
//

int fitScreen(int x, int y)
{
    int i=0;
    int j=1;
    struct node* nod=head;
    if (!nod) return(0);
    while(nod)
    {
        i++;
        if ((i%7 == 0) && (nod->next)) j+=1+(nod->length);
        nod=nod->next;
    }
    if ((j+(nod->length)) < x ) return(1);
    else return(0);
}


int Length(struct node* head)
{

    struct node* current = head;
    int count = 0;

    while (current != NULL)
    {
        //printf("%s\n", current->anagram);
        count++;
        current = current->next;
    }

    return count;
}


void newGame()
{
    char* guess;
    char* remain;
    int i;
    struct node* nod;
    echof(0,"starting new game.\n");
    guess = malloc(sizeof(char)*50);
    remain = malloc(sizeof(char)*50);
    do
    {
        strcpy(guess,"\0");
        debug(2,"rootWord: %s\n",getRandomWord());
        strcpy(rootWord, getRandomWord());
        bigWordLen = strlen(rootWord)-1;
        strcpy(remain,rootWord);
        rootWord[bigWordLen] = '\0';
        destroyAnswers(&head);
        debug(2,"generate anagrams from random word \n");
        ag(guess, remain);
        sort(&head);
        answersSought = Length(head);
        debug(2,"rootWord: %s, answers:%d\n",rootWord,Length(head));
    }
    while( (answersSought > 80) | (answersSought < 6));

    for (i=bigWordLen;i<lettersNum;i++)
    {
        remain[i]=' ';
    }

    remain[lettersNum] = '\0';

    //	printf("%s,%i\n", remain, bigWordLen);
    remain[bigWordLen]='\0';

    shuffleString(remain);
    sort(&head);
    free(guess);
    free(remain);
    //#ifdef DEBUG
    nod=head;
    while(nod)
    {
        nod=nod->next;
    }
    //#endif

}


void solveIt(struct node* head)
{
    struct node* current = head;
    while(current != NULL)
    {
                                 //guessed by the server...
        if (!current->guessed) current->guessed = MAX_PLAYERS+1;
        current = current->next;
    }
}
