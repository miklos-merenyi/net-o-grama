#include <SDL2/SDL.h>
#include <ctype.h>
#include <string.h>
#include "cli_sdl.h"
#include "client.h"

extern int nop;
extern struct gamer gamers[MAX_PLAYERS];

int key_SHUFFLE = ' ';
int key_CHECK = '\n';
int key_CHECK_NORET = SDLK_RIGHT;
int key_CLEAR = SDLK_BACKSPACE;
int key_DELCHAR = SDLK_LEFT;
int key_SOLVE = '\t';
int key_QUIT = 27;

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 720
#define PLAY_X 20
#define PLAY_Y 20
#define PLAY_W 700
#define PLAY_H 220
#define SCORE_X 720
#define SCORE_Y 20
#define SCORE_W 304
#define SCORE_H 220
#define GUESS_X 20
#define GUESS_Y 260
#define GUESS_W 980
#define GUESS_H 440

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static char currentWord[16] = "";
static char currentTry[16] = "";
static int currentTime = 0;
static struct node *currentHead = NULL;

// Color palette for 4 players (matching ncurses colors)
static SDL_Color playerColors[4] = {
    {255, 0, 0, 255},     // Player 0: Red
    {0, 255, 0, 255},     // Player 1: Green
    {255, 255, 0, 255},   // Player 2: Yellow
    {0, 255, 255, 255}    // Player 3: Cyan
};

static const uint8_t sdl_font[][8] = {
    {0x18,0x24,0x42,0x42,0x7E,0x42,0x42,0x42}, // A
    {0x7C,0x42,0x42,0x7C,0x42,0x42,0x42,0x7C}, // B
    {0x3C,0x42,0x40,0x40,0x40,0x40,0x42,0x3C}, // C
    {0x78,0x44,0x42,0x42,0x42,0x42,0x44,0x78}, // D
    {0x7E,0x40,0x40,0x7C,0x40,0x40,0x40,0x7E}, // E
    {0x7E,0x40,0x40,0x7C,0x40,0x40,0x40,0x40}, // F
    {0x3C,0x42,0x40,0x4E,0x42,0x42,0x42,0x3C}, // G
    {0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x42}, // H
    {0x3C,0x18,0x18,0x18,0x18,0x18,0x18,0x3C}, // I
    {0x1E,0x08,0x08,0x08,0x08,0x48,0x48,0x30}, // J
    {0x42,0x44,0x48,0x70,0x48,0x44,0x42,0x42}, // K
    {0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x7E}, // L
    {0x42,0x66,0x5A,0x5A,0x42,0x42,0x42,0x42}, // M
    {0x42,0x62,0x52,0x4A,0x46,0x42,0x42,0x42}, // N
    {0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x3C}, // O
    {0x7C,0x42,0x42,0x7C,0x40,0x40,0x40,0x40}, // P
    {0x3C,0x42,0x42,0x42,0x42,0x4A,0x44,0x3A}, // Q
    {0x7C,0x42,0x42,0x7C,0x48,0x44,0x42,0x42}, // R
    {0x3C,0x42,0x40,0x3C,0x02,0x02,0x42,0x3C}, // S
    {0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x18}, // T
    {0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x3C}, // U
    {0x42,0x42,0x42,0x42,0x42,0x42,0x24,0x18}, // V
    {0x42,0x42,0x42,0x42,0x5A,0x5A,0x66,0x42}, // W
    {0x42,0x42,0x24,0x18,0x18,0x24,0x42,0x42}, // X
    {0x42,0x42,0x42,0x24,0x18,0x18,0x18,0x18}, // Y
    {0x7E,0x02,0x04,0x08,0x10,0x20,0x40,0x7E}, // Z
    {0x3C,0x42,0x62,0x52,0x4A,0x46,0x42,0x3C}, // 0
    {0x18,0x38,0x18,0x18,0x18,0x18,0x18,0x3C}, // 1
    {0x3C,0x42,0x02,0x04,0x08,0x10,0x20,0x7E}, // 2
    {0x3C,0x42,0x02,0x1C,0x02,0x02,0x42,0x3C}, // 3
    {0x04,0x0C,0x14,0x24,0x44,0x7E,0x04,0x04}, // 4
    {0x7E,0x40,0x40,0x7C,0x02,0x02,0x42,0x3C}, // 5
    {0x3C,0x40,0x40,0x7C,0x42,0x42,0x42,0x3C}, // 6
    {0x7E,0x02,0x04,0x08,0x10,0x10,0x10,0x10}, // 7
    {0x3C,0x42,0x42,0x3C,0x42,0x42,0x42,0x3C}, // 8
    {0x3C,0x42,0x42,0x42,0x3E,0x02,0x02,0x3C}, // 9
    {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00}, // :
    {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00}, // -
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00}, // .
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}  // space
};

static const char sdl_font_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:-. ";

static const uint8_t *getGlyph(char c)
{
    char key = (char) toupper((unsigned char)c);
    for (size_t i = 0; i < sizeof(sdl_font_chars) - 1; ++i)
    {
        if (sdl_font_chars[i] == key)
            return sdl_font[i];
    }
    return sdl_font[sizeof(sdl_font_chars) - 2];
}

static void drawText(SDL_Renderer *renderer, int x, int y, const char *text, SDL_Color color, int scale)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (size_t i = 0; i < strlen(text); ++i)
    {
        const uint8_t *glyph = getGlyph(text[i]);
        for (int row = 0; row < 8; ++row)
        {
            for (int col = 0; col < 8; ++col)
            {
                if (glyph[row] & (1 << (7 - col)))
                {
                    SDL_Rect pixel = {x + col * scale, y + row * scale, scale, scale};
                    SDL_RenderFillRect(renderer, &pixel);
                }
            }
        }
        x += (8 + 1) * scale;
    }
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}

static void drawPanel(SDL_Renderer *renderer, const SDL_Rect *rect, SDL_Color bg, SDL_Color border)
{
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRect(renderer, rect);
    SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, border.a);
    SDL_RenderDrawRect(renderer, rect);
}

static void renderScreen()
{
    SDL_Color bg = {12, 18, 32, 255};
    SDL_Color panel = {22, 36, 56, 255};
    SDL_Color border = {100, 170, 230, 255};
    SDL_Color text = {240, 240, 240, 255};
    SDL_Color accent = {200, 170, 90, 255};

    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderClear(renderer);

    SDL_Rect playRect = {PLAY_X, PLAY_Y, PLAY_W, PLAY_H};
    SDL_Rect scoreRect = {SCORE_X, SCORE_Y, SCORE_W, SCORE_H};
    SDL_Rect guessRect = {GUESS_X, GUESS_Y, GUESS_W, GUESS_H};

    drawPanel(renderer, &playRect, panel, border);
    drawPanel(renderer, &scoreRect, panel, border);
    drawPanel(renderer, &guessRect, panel, border);

    drawText(renderer, PLAY_X + 16, PLAY_Y + 18, currentWord, text, 5);
    drawText(renderer, PLAY_X + 16, PLAY_Y + 120, currentTry, accent, 5);

    char timeBuffer[16];
    snprintf(timeBuffer, sizeof(timeBuffer), "TIME:%3d", currentTime);
    drawText(renderer, SCORE_X + 10, SCORE_Y + 15, timeBuffer, accent, 2);

    for (int i = 0; i < nop; ++i)
    {
        int y = SCORE_Y + 15 + (i + 1) * 38;
        SDL_Color playerColor = playerColors[i % 4];
        char nameDisplay[9];
        strncpy(nameDisplay, gamers[i].name, 9); 
        nameDisplay[8] = '\0';
        drawText(renderer, SCORE_X + 10, y, nameDisplay, playerColor, 2);
        drawText(renderer, SCORE_X + 154, y, ":", playerColor, 2);
        char scoreText[16];
        snprintf(scoreText, sizeof(scoreText), "%3d", gamers[i].score);
        drawText(renderer, SCORE_X + 180, y, scoreText, playerColor, 2);
    }

    if (currentHead)
    {
        const struct node *nod = currentHead;
        int col = 0;
        int row = 0;
        const int guessScale = 2;
        const int guessRowHeight = 26;
        const int guessColWidth = 160;
        const int maxRows = (GUESS_H - 20) / guessRowHeight;

        while (nod)
        {
            int x = GUESS_X + col * guessColWidth + 10;
            int y = GUESS_Y + row * guessRowHeight + 10;
            if (nod->guessed > 0)
            {
                SDL_Color guessColor = playerColors[(nod->guessed - 1) % 4];
                drawText(renderer, x, y, nod->anagram, guessColor, guessScale);
            }
            else
            {
                char line[12] = {0};
                for (int j = 0; j < nod->length; ++j) line[j] = '-';
                drawText(renderer, x, y, line, accent, guessScale);
            }

            row++;
            if (row >= maxRows)
            {
                row = 0;
                col++;
            }
            nod = nod->next;
        }
    }

    SDL_RenderPresent(renderer);
}

void initScreen()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        exit(1);
    }
    window = SDL_CreateWindow("Net-o-Grama SDL2 Client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
    {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }
    memset(currentWord, 0, sizeof(currentWord));
    memset(currentTry, 0, sizeof(currentTry));
    currentTime = 0;
    currentHead = NULL;
    renderScreen();
}

void endScreen()
{
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

void updatePlayField(char *Word, char *Try, int new)
{
    strncpy(currentWord, Word, sizeof(currentWord) - 1);
    strncpy(currentTry, Try, sizeof(currentTry) - 1);
    currentWord[sizeof(currentWord) - 1] = '\0';
    currentTry[sizeof(currentTry) - 1] = '\0';
    renderScreen();
}

void displayMessage(char *Line1, char *Line2)
{
    updatePlayField(Line1, Line2, 1);
}

void initPlayField(char *Word)
{
    updatePlayField(Word, "       ", 1);
}

void updateTime(int time)
{
    currentTime = time;
    renderScreen();
}

void drawScoreBoard(int refr)
{
    if (refr) renderScreen();
}

void drawGuessBoard(struct node* head, int refr)
{
    currentHead = head;
    if (refr) renderScreen();
}

void handle_input(SDL_Event *event)
{
    if (event->type == SDL_QUIT)
    {
        sendf(srv, "quit");
        exit(0);
    }
    if (event->type != SDL_KEYDOWN) return;
    SDL_Keycode key = event->key.keysym.sym;
    int mapped = 0;
    if (key == SDLK_ESCAPE) mapped = key_QUIT;
    else if (key == SDLK_SPACE) mapped = key_SHUFFLE;
    else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) mapped = key_CHECK;
    else if (key == SDLK_RIGHT) mapped = key_CHECK_NORET;
    else if (key == SDLK_LEFT) mapped = key_DELCHAR;
    else if (key == SDLK_BACKSPACE) mapped = key_CLEAR;
    else if (key == SDLK_TAB) mapped = key_SOLVE;
    else if (key >= SDLK_a && key <= SDLK_z) mapped = (int) key;
    else if (key >= SDLK_1 && key <= SDLK_7) mapped = '1' + (key - SDLK_1);
    if (mapped) keyPressed(mapped);
}

void gameLoop()
{
    while (1)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            handle_input(&event);
        }

        fd_set rd;
        struct timeval tv;
        int r;

        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        FD_ZERO(&rd);
        FD_SET(srv, &rd);
        r = select(srv + 1, &rd, NULL, NULL, &tv);
        if (r < 0) error(1, 0, "select()");
        if (FD_ISSET(srv, &rd)) recv_msg();
        heartBeat();
        SDL_Delay(10);
    }
}
