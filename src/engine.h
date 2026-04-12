// ASCII OFFSET to convert a number to it's character equivalent
#define NUM_TO_CHAR 48
#define ASCII_SPACE 32
extern char *wordlist;
void newGame();
void solveIt(struct node* head);
void shuffleString(char* thisWord);
int utf8_strlen(const char* str);  // Count UTF-8 characters, not bytes
