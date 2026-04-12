#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include <stdio.h>

struct node
{
    char    anagram[64]; // Increased for UTF-8
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
    unsigned char search = (unsigned char)letter;
    int i;
    
    // For single-byte ASCII characters only
    if (search < 0x80) {
        for(i = 0; i <= (int)strlen(string); i++) {
            if ((unsigned char)string[i] == search)
                return i;
        }
        return(-1);
    }
    
    // Multi-byte UTF-8 should use whereinstr_utf8() instead
    return(-1);
}

// Search for a complete UTF-8 character (multi-byte sequence) in a string
// utf8_bytes: buffer containing the complete UTF-8 character bytes
// utf8_len: number of bytes in the UTF-8 character (e.g., 2 for 'í')
// Returns: position of the first matching sequence, or -1 if not found
int whereinstr_utf8(char* string, const char* utf8_bytes, int utf8_len)
{
    if (!string || !utf8_bytes || utf8_len <= 0 || utf8_len > 4) {
        return -1;
    }
    
    int string_len = strlen(string);
    
    // Search for the complete UTF-8 byte sequence
    for (int i = 0; i < string_len; i++) {
        // Check if we have enough remaining bytes to match
        if (i + utf8_len > string_len) {
            break;
        }
        
        // Compare all bytes of the UTF-8 sequence
        int match = 1;
        for (int j = 0; j < utf8_len; j++) {
            if ((unsigned char)string[i + j] != (unsigned char)utf8_bytes[j]) {
                match = 0;
                break;
            }
        }
        
        if (match) {
            return i;  // Found the complete sequence
        }
    }
    
    return -1;  // Not found
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
