#define AVAILABLE_TIME 3
#define MAX_PLAYERS 4
#define TIMEOUT 5
#define BEFORE 0                 //before game
#define GETID 1                  //get ID from server
#define GETNAMES 2               //get names
#define GETNODES 3               //get nodes
#define GETSHUFFLE 4             //get shuffled word
#define COUNTDOWN 5              //3..2..1
#define GAMEISON 6               //game is on
#define WAITING 7                //game is on, waiting for approve for a guess
#define ENDED 8                  //game is ended

extern int debugLevel;
extern char * logfile;
struct node
{
    char    anagram[10];
    int     found;
    int     guessed;
    int     length;
    int     id;
    struct node* next;
};

void destroyAnswers(struct node** headRef);
int whereinstr(char* string, char letter);
void nextWord(char* to, char *string, int *pos);
