#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include <stdio.h>

struct node
{
    char    anagram[10];
    int     found;
    int     guessed;
    int     length;
    int     id;
    struct node* next;
};

void nextWord(char* to, char *string, int *pos)
{
    int i,len;
    //debug("nextWord(%s, %d)",&string[*pos],*pos);
    len=strlen(string);
    if ((len<*pos) || (*pos<0))
    {
        to[0]='\0';
        return;
    }
    i=*pos;
    while(string[i]!=':' && i<len) i++;
    strncpy(to, &string[*pos],i-*pos);
    to[i-*pos]='\0';
    if(i==len) *pos=-1; else *pos=i+1;
}


int whereinstr(char* string, char letter)
{
    int i;
    for(i=0;i<=strlen(string);i++)
    {
        if (string[i]==letter)
            return i;
    }
    return(-1);
}


void destroyAnswers(struct node** headRef)
{
    // destroy the whole answers list
    debug(5,"Destroying answer list\n");
    struct node* current = *headRef;
    struct node* next=NULL;

    while (current != NULL)
    {
        next = current->next;
        debug(6,"Freeing up \"%s\"\n",current->anagram);
        free(current->anagram);
        debug(6,"Freeing its node\n");
        //		free(current);
        current = next;
    }

    *headRef = NULL;
}
