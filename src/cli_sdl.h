#ifndef CLI_SDL_H
#define CLI_SDL_H

#include <SDL2/SDL.h>

struct node;

void initScreen();
void endScreen();
void initPlayField(char *Word);
void updatePlayField(char *Word, char *Try, int new);
void displayMessage(char *Line1, char *Line2);
void drawScoreBoard(int refr);
void drawGuessBoard(struct node* head, int refr);
void updateTime(int refr);
void gameLoop();
void handle_input(SDL_Event *event);

#endif // CLI_SDL_H
