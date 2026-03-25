#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void updatePlayField(char *Word, char *Try, int new);
void initPlayField(char *Word);
void displayMessage(char *Line1, char *Line2);
void drawScoreBoard(int refr);
void drawGuessBoard(struct node* head, int refr);
void initScreen();
void endScreen();
void updateTime(int refr);
void gameLoop();
