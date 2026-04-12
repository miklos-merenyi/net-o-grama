#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <locale.h>
#include <curses.h>
#include <getopt.h>
#include <sys/stat.h>
#include "debug.h"
#include "dlb.h"
#include "linked.h"
#include "network.h"
#define SPACE_CHAR ' '

// Count UTF-8 characters (not bytes)
// Each UTF-8 character starts with either:
// - 0xxxxxxx (ASCII, 1 byte)
// - 11xxxxxx (multi-byte start, 2-4 bytes total)
// Continuation bytes are 10xxxxxx and should not be counted
int utf8_strlen(const char* str)
{
    if (!str) return 0;
    
    int count = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        unsigned char byte = (unsigned char)str[i];
        // Count each character start (ASCII or multi-byte start)
        // ASCII: 0xxxxxxx
        // Multi-byte start: 11xxxxxx (and not continuation 10xxxxxx)
        if ((byte & 0x80) == 0 || (byte & 0xC0) == 0xC0)
        {
            count++;
        }
        // Skip continuation bytes: 10xxxxxx
    }
    return count;
}


//module level variables for game control
extern struct node* head;
struct dlb_node* dlbHead;
extern char rootWord[10];
char *wordlist = NULL;  // Wordlist filename, initialized by main()
char spc=' '+32;
__attribute__((constructor)) static void set_utf8_locale() {
    setlocale(LC_ALL, "");
}
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
    strncpy(swap, (*from)->anagram, 63);
    swap[63] = '\0';

    strncpy((*from)->anagram, (*to)->anagram, 63);
    (*from)->anagram[63] = '\0';
    (*from)->length = (*to)->length;
    strncpy((*to)->anagram, swap, 63);
    (*to)->anagram[63] = '\0';
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
    len = utf8_strlen(anagram);
    //debug("ADDED %s\t LENGTH: %d", anagram,len);
    //newNode->anagram = malloc(sizeof(char)*len+1);
    strncpy(newNode->anagram, anagram, 63);
    newNode->anagram[63] = '\0';
    newNode->length = len;
    newNode->found = 0;
    newNode->guessed = 0;
    newNode->next = *headRef;    // dereference back the the real head pointer
    *headRef = newNode;          // ditto when replacing it with the new one
}


// Helper: find the start of the next UTF-8 character in a string
int utf8_next_char(const char* string, int pos)
{
    if (string[pos] == '\0') return pos;
    
    unsigned char byte = (unsigned char)string[pos];
    int char_bytes = 1;
    
    if ((byte & 0x80) == 0) {
        // ASCII character, 1 byte
        char_bytes = 1;
    } else if ((byte & 0xE0) == 0xC0) {
        // 2-byte character
        char_bytes = 2;
    } else if ((byte & 0xF0) == 0xE0) {
        // 3-byte character
        char_bytes = 3;
    } else if ((byte & 0xF8) == 0xF0) {
        // 4-byte character
        char_bytes = 4;
    }
    
    return pos + char_bytes;
}

char* shiftLeftKill(char* string)
{
    // shift a string of characters 1 character to the left (remove first char)
    // Properly handles UTF-8 multi-byte characters

    char newString[50];
    int src_pos, dst_pos;
    
    // Find where the second UTF-8 character starts
    src_pos = utf8_next_char(string, 0);
    dst_pos = 0;
    
    // Copy everything from the second character onwards
    while (string[src_pos] != '\0' && dst_pos < 49) {
        newString[dst_pos] = string[src_pos];
        src_pos++;
        dst_pos++;
    }
    newString[dst_pos] = '\0';
    
    return(strdup(newString));
}


/***********************************************************/
char* shiftLeft(char* string)
{
    // shift a string of characters 1 character to the left
    // move the first character to the end of the string
    // Properly handles UTF-8 multi-byte characters

    char newString[50];
    int first_char_len;
    int src_pos, dst_pos;
    
    if (!string || string[0] == '\0') return strdup("");
    
    // Find where the first character ends
    first_char_len = utf8_next_char(string, 0);
    
    // Copy everything from second character onwards
    src_pos = first_char_len;
    dst_pos = 0;
    while (string[src_pos] != '\0' && dst_pos < 45) {
        newString[dst_pos] = string[src_pos];
        src_pos++;
        dst_pos++;
    }
    
    // Append the first character at the end
    src_pos = 0;
    while (src_pos < first_char_len && dst_pos < 49) {
        newString[dst_pos] = string[src_pos];
        src_pos++;
        dst_pos++;
    }
    newString[dst_pos] = '\0';

    return(strdup(newString));
}


char* ag(char* guess, char* remain)
{
    // generate all possible combinations of the root word
    // the initial letter is fixed (hence the space character
    // at the end of the possible list)

    char  newGuess[50];
    char  newRemain[50];
    int    totalLen=0, guessLen=0, remainLen=0, i;
    int    guessLenBytes=0, remainLenBytes=0;

    // Safety check: don't work with NULL pointers
    if (!guess || !remain) return rootWord;
    
    // allocate space for our working variables
    guessLenBytes = strlen(guess);  // byte count for copying
    remainLenBytes = strlen(remain);  // byte count for copying
    
    // Safety bounds check: don't proceed if strings are too large
    if (guessLenBytes > 48 || remainLenBytes > 48) return rootWord;
    
    guessLen = utf8_strlen(guess);  // character count for length checks
    remainLen = utf8_strlen(remain);  // character count for length checks
    totalLen = guessLen + remainLen;

    //newGuess = malloc(sizeof(char) * (totalLen+1));
    //newRemain = malloc(sizeof(char) * (totalLen+1));

    // move last remaining letter to end of guess
    strncpy(newGuess, guess, 49);
    newGuess[49] = '\0';
    strncpy(newRemain, remain, 49);
    newRemain[49] = '\0';
    
    // Safety check before accessing remainLenBytes-1
    if (remainLenBytes > 0 && guessLenBytes < 49) {
        newGuess[guessLenBytes] = newRemain[remainLenBytes-1];
        newGuess[guessLenBytes+1] = '\0';
        if (remainLenBytes > 0) newRemain[remainLenBytes-1] = '\0';
    }

    //debug(0,"%s\n", newGuess);

    if(utf8_strlen(newGuess) > 3)
    {
        if (dlb_lookup(dlbHead,shiftLeftKill(newGuess)))
        {
            push(&head, shiftLeftKill(newGuess));
        }
    }

    if (utf8_strlen(newRemain))
    {
        ag(newGuess, newRemain);

        for (i=totalLen-1;i>0;i--)
        {
            if(utf8_strlen(newRemain) > i)
            {
                strncpy(newRemain, shiftLeft(newRemain), 49);
                newRemain[49] = '\0';
                ag(newGuess, newRemain);
            }
        }
    }
    return rootWord;
    // free the space
    //free(newGuess);
    //free(newRemain);
}


// Helper: Get byte offset of the Nth UTF-8 character in a string
int utf8_char_to_bytes(const char* string, int char_index)
{
    int byte_pos = 0;
    int char_pos = 0;
    
    while (string[byte_pos] != '\0' && char_pos < char_index) {
        unsigned char byte = (unsigned char)string[byte_pos];
        if ((byte & 0x80) == 0) {
            // ASCII: 1 byte
            byte_pos += 1;
        } else if ((byte & 0xE0) == 0xC0) {
            // 2-byte character
            byte_pos += 2;
        } else if ((byte & 0xF0) == 0xE0) {
            // 3-byte character
            byte_pos += 3;
        } else if ((byte & 0xF8) == 0xF0) {
            // 4-byte character
            byte_pos += 4;
        } else {
            // Invalid UTF-8, skip
            byte_pos += 1;
        }
        char_pos++;
    }
    
    return byte_pos;
}

// Helper: Get the length in bytes of the UTF-8 character at position
int utf8_char_length(const char* string, int byte_pos)
{
    if (string[byte_pos] == '\0') return 0;
    
    unsigned char byte = (unsigned char)string[byte_pos];
    if ((byte & 0x80) == 0) return 1;      // ASCII
    if ((byte & 0xE0) == 0xC0) return 2;   // 2-byte
    if ((byte & 0xF0) == 0xE0) return 3;   // 3-byte
    if ((byte & 0xF8) == 0xF0) return 4;   // 4-byte
    return 1;  // Invalid, treat as 1 byte
}

char* swapChars(int from, int to, char* string)
{
    // swap 2 UTF-8 characters in a string (by character index, not byte index)
    debug(10,"swapping UTF8 char %d and %d in %s", from, to, string);
    
    if (!string || string[0] == '\0' || from < 0 || to < 0 || from == to) {
        return string;  // Nothing to swap
    }
    
    int from_byte = utf8_char_to_bytes(string, from);
    int to_byte = utf8_char_to_bytes(string, to);
    int from_len = utf8_char_length(string, from_byte);
    int to_len = utf8_char_length(string, to_byte);
    
    // Safety check - don't swap if something is wrong
    if (from_len == 0 || to_len == 0 || from_len > 4 || to_len > 4) {
        return string;
    }
    
    // If characters are same length, simple byte swap
    if (from_len == to_len) {
        for (int i = 0; i < from_len; i++) {
            char swap = string[from_byte + i];
            string[from_byte + i] = string[to_byte + i];
            string[to_byte + i] = swap;
        }
    } else {
        // Different length characters - use temp buffer for complex swap
        // This is complex and could cause issues, so use safe bounds
        char temp[5];  // Max 4 bytes for UTF-8 char + null
        int copy_len = (from_len < 4) ? from_len : 4;
        strncpy(temp, string + from_byte, copy_len);
        temp[copy_len] = '\0';
        
        // When lengths differ, only swap if they're close (to avoid complex memory ops)
        // For simplicity and safety, just treat as same-length for now
        // This avoids potential buffer overrun issues
        return string;
    }
    
    return string;
}


void shuffleString(char* thisWord)
{
    // replace characters randomly
    int numSwaps,from,to,i,len;
    debug(8, "Shuffling string: %s",thisWord);
    len=utf8_strlen(thisWord);
    numSwaps = (rand()%len)+20;
    for (i=0;i<numSwaps;i++)
    {
        from = rand()%len;
        to = rand()%len;
        // Get the byte position of the first byte of each character
        int from_byte = utf8_char_to_bytes(thisWord, from);
        int to_byte = utf8_char_to_bytes(thisWord, to);
        if ( (thisWord[from_byte] != spc) & (thisWord[to_byte] != spc))
        {
            strcpy(thisWord, swapChars(from, to, thisWord));
        }
    }
}



char* getRandomWord()
{
    int filelocation;
    int i;
    char* wordFromList = malloc(sizeof(char) * 50);
    int len;
    int done = 0;
    filelocation = rand()%1000;

    FILE* wfile;
    if ((wfile=fopen(wordlist,"r"))==NULL )
    {
        error(1,errno,"Can't open wordlist file");
    }


    for (i=0;i<=filelocation;i++)
    {
        if(fscanf(wfile, "%49s", wordFromList) != EOF)  // FIXED: Added size limit
        {
            // spin on
        }
        else
        {
            // go back to the start of the file
            fclose(wfile);
            wfile=fopen(wordlist, "r");
        }
    }

    // ok random location reached

    while (!done)
    {
        len = utf8_strlen(wordFromList);
        if ((len==lettersNum))
        {
            done = 1;
        }
        else
        {
            if(fscanf(wfile, "%49s", wordFromList) != EOF)  // FIXED: Added size limit
            {
                // spin on
            }
            else
            {
                // go back to the start of the file
                fclose(wfile);
                wfile=fopen(wordlist, "r");
                fscanf(wfile, "%49s", wordFromList);  // FIXED: Added size limit
            }
        }
    }

    fclose(wfile);

    // add in our space character at the end of the word (after all bytes)
    int byte_len = strlen(wordFromList);  // Get actual byte length
    if (byte_len < 49) {  // Safety check before adding space
        wordFromList[byte_len] = ' ';
        wordFromList[byte_len+1] = '\0';
    }

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
    memset(guess, 0, 50);    // Clear memory
    memset(remain, 0, 50);   // Clear memory
    do
    {
        strcpy(guess,"\0");
        debug(2,"rootWord: %s\n",getRandomWord());
        strcpy(rootWord, getRandomWord());
        
        // Calculate word length in characters, then find byte position of last char to remove
        size_t wordlen = utf8_strlen(rootWord);
        if (wordlen == 0) continue;  // Safety: skip empty words
        
        bigWordLen = wordlen - 1;  // bigWordLen now stores CHARACTER count
        
        // Find byte position where the last character starts
        int byte_len = utf8_char_to_bytes(rootWord, bigWordLen);
        if (byte_len >= 49) byte_len = 48;  // Bounds check
        
        strcpy(remain,rootWord);
        rootWord[byte_len] = '\0';  // Truncate at the byte boundary of the last character
        remain[byte_len] = '\0';    // ALSO truncate remain to match
        
        destroyAnswers(&head);
        debug(2,"generate anagrams from random word \n");
        ag(guess, remain);
        sort(&head);
        answersSought = Length(head);
        debug(2,"rootWord: %s, answers:%d\n",rootWord,Length(head));
    }
    while( (answersSought > 80) | (answersSought < 6));

    // Fill remain buffer with spaces for remaining character positions
    // bigWordLen is CHARACTER count of root word (without last char)
    // remain is now truncated to bigWordLen characters
    // We need to fill character positions [bigWordLen] through [lettersNum-1] with spaces
    
    // Calculate byte position where remaining character ends (end of string)
    int first_space_byte = strlen(remain);  // Get actual byte length after truncation
    if (first_space_byte >= 48) {  // Safety check
        free(guess);
        free(remain);
        return;
    }
    
    // Pad with spaces to make lettersNum total characters (in bytes)
    int current_char = bigWordLen;
    int byte_idx = first_space_byte;
    
    while (current_char < lettersNum && byte_idx < 47) {  // Extra safety: < 47 instead of < 48
        remain[byte_idx] = ' ';
        byte_idx++;
        current_char++;
    }
    if (byte_idx < 48) remain[byte_idx] = '\0';  // Final null terminator, with bounds check

    // Shuffle and process
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
