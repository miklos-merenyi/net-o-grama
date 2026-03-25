#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dlb.h"
#include "debug.h"

/***************************************************************/
struct dlb_node* dlb_insertLetter(char thisLetter)
{
    // create and return a new letter node
    struct dlb_node* newNode = malloc(sizeof(struct dlb_node));

    //	printf("dbl_insertLetter %c\n", thisLetter);

    newNode->letter = thisLetter;
    newNode->valid = 0;
    newNode->sibling = NULL;
    newNode->child = NULL;

    return newNode;
}


/***************************************************************/
void dlb_push(struct dlb_node** dlbHead, char* thisWord)
{
    // add a new word to the dictionary
    struct dlb_node* current = *dlbHead;
    struct dlb_node* previous = NULL;
    int i=0;
    char letter;
    int child = 0;
    int sibling = 0;
    int newHead = (*dlbHead==NULL);

    //	printf("dbl_push %s\n", thisWord);
    //	printf("head : %i\n", newHead);

    while (i<=strlen(thisWord)-1)
    {

        letter = thisWord[i];

        if (current == NULL)
        {
            current = dlb_insertLetter(letter);
            if (newHead)
            {
                *dlbHead = current;
                newHead = 0;
            }
            if (child)
            {
                previous->child = current;
            }
            if (sibling)
            {
                previous->sibling = current;
            }
        }

        child = 0;
        sibling = 0;

        previous = current;

        if (letter == previous->letter)
        {
            i++;
            child = 1;
            current = previous->child;
        }
        else
        {
            sibling = 1;
            current = previous->sibling;
        }
    }
    previous->valid = 1;
}


/***************************************************************/
void createDLBTree(struct dlb_node** dlbHead, char* wordlist_file)
{
    // open the wordlist file and push all words onto the dictionary

    FILE* wordlist;
    char wordFromList[50];

    //printf("createDLBTree\n");

    // open wordlist file
    wordlist = fopen(wordlist_file, "r");
    if (!wordlist) error(1,errno,"error opening wordlist.txt");

    // get each word from the list
    while (fscanf(wordlist, "%s", wordFromList) != EOF)
    {
        dlb_push(&(*dlbHead),wordFromList);
    }

    // close wordlist file
    fclose(wordlist);
}


/***************************************************************/
int dlb_lookup(struct dlb_node* dlbHead, char* thisWord)
{
    // determine if a given word is in the dictionary
    // essentially the same as a push, but doesn't add
    // any of the new letters

    struct dlb_node* current = dlbHead;
    struct dlb_node* previous = NULL;
    int i=0;
    char letter;
    int retval = 0;

    //printf("dlb_lookup %s\n", thisWord);

    while (i<=strlen(thisWord)-1)
    {

        if (current == NULL)
        {
            retval = 0;
            break;
        }

        //printf("%c",current->letter);

        letter = thisWord[i];

        previous = current;

        if (letter == previous->letter)
        {
            i++;
            current = previous->child;
            //printf(".%c., %i\n",previous->letter,previous->valid);
            retval = previous->valid;
        }
        else
        {
            current = previous->sibling;
            retval = 0;
        }
    }

    return retval;
}
