#include <stdlib.h>

struct dlb_node
{

    char letter;
    int valid;
    struct dlb_node* sibling;
    struct dlb_node* child;
};

struct dlb_node* dlb_insertLetter(char thisLetter);
void dlb_push(struct dlb_node** dlbHead, char* thisWord);
void createDLBTree(struct dlb_node** dlbHead, char* wordlist_file);
int dlb_lookup(struct dlb_node* dlbHead, char* thisWord);
