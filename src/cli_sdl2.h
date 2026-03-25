#ifndef CLI_SDL2_H
#define CLI_SDL2_H

#include <SDL2/SDL.h>

// Function declarations
void init_sdl();
void render_board(SDL_Renderer *renderer);
void update_scoreboard(SDL_Renderer *renderer, int score);
void handle_input(SDL_Event *event);

#endif // CLI_SDL2_H
