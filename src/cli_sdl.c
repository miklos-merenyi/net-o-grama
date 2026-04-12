#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
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
#define SCORE_W 280
#define SCORE_H 220
#define GUESS_X 20
#define GUESS_Y 260
#define GUESS_W 980
#define GUESS_H 440

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static TTF_Font *font = NULL;
static char currentWord[64] = "";  // Increased for UTF-8 multi-byte characters
static char currentTry[64] = "";   // Increased for UTF-8 multi-byte characters
static int currentTime = 0;
static struct node *currentHead = NULL;

// Color palette for 4 players (matching ncurses colors)
static SDL_Color playerColors[4] = {
    {255, 0, 0, 255},     // Player 0: Red
    {0, 255, 0, 255},     // Player 1: Green
    {255, 255, 0, 255},   // Player 2: Yellow
    {0, 255, 255, 255}    // Player 3: Cyan
};

// TTF-based text rendering (UTF-8 support)
static void drawText(SDL_Renderer *renderer, int x, int y, const char *text, SDL_Color color, int scale)
{
    if (!font || !text || strlen(text) == 0) return;
    
    SDL_Surface *textSurface = TTF_RenderUTF8_Solid(font, text, color);
    if (!textSurface) return;
    
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        SDL_FreeSurface(textSurface);
        return;
    }
    
    // Properly scale the text without distortion
    int destW = textSurface->w;
    int destH = textSurface->h;
    if (scale > 1) {
        destW = (textSurface->w * scale + 50) / 100;  // Scale as percentage
        destH = (textSurface->h * scale + 50) / 100;
    }
    SDL_Rect destRect = {x, y, destW, destH};
    SDL_RenderCopy(renderer, textTexture, NULL, &destRect);
    
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
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

    drawText(renderer, PLAY_X + 16, PLAY_Y + 18, currentWord, text, 150);  // 150% scale
    drawText(renderer, PLAY_X + 16, PLAY_Y + 120, currentTry, accent, 150);

    char timeBuffer[16];
    snprintf(timeBuffer, sizeof(timeBuffer), "TIME:%3d", currentTime);
    drawText(renderer, SCORE_X + 10, SCORE_Y + 15, timeBuffer, accent, 90);  // 90% scale - smaller

    for (int i = 0; i < nop; ++i)
    {
        int y = SCORE_Y + 15 + (i + 1) * 38;
        SDL_Color playerColor = playerColors[i % 4];
        char nameDisplay[9];
        const char *nameStart = gamers[i].name;
        while (*nameStart == ' ' && *nameStart != '\0') nameStart++;
        strncpy(nameDisplay, nameStart, 8);
        nameDisplay[8] = '\0';
        drawText(renderer, SCORE_X + 10, y, nameDisplay, playerColor, 90);
        drawText(renderer, SCORE_X + 154, y, ":", playerColor, 90);
        char scoreText[16];
        snprintf(scoreText, sizeof(scoreText), "%3d", gamers[i].score);
        drawText(renderer, SCORE_X + 180, y, scoreText, playerColor, 90);
    }

    if (currentHead)
    {
        const struct node *nod = currentHead;
        int col = 0;
        int row = 0;
        const int guessScale = 90;  // 90% scale (percentage format) - smaller
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
    if (TTF_Init() != 0)
    {
        fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        exit(1);
    }
    window = SDL_CreateWindow("Net-o-Grama SDL2 Client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
    {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }
    
    // Load a fixed-width (monospace) font for proper text alignment
    // DejaVuSansMono and LiberationMono support UTF-8 and Hungarian characters
    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 28);
    if (!font)
    {
        font = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf", 28);
    }
    if (!font)
    {
        // Try alternative monospace fonts
        font = TTF_OpenFont("/usr/share/fonts/truetype/courier/courR8092.ttf", 28);
    }
    if (!font)
    {
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf", 26);
    }
    if (!font)
    {
        // Fall back to proportional fonts if monospace unavailable
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 26);
    }
    if (!font)
    {
        fprintf(stderr, "Warning: Could not load TTF font, using built-in rendering\n");
    }
    
    // Enable text input for UTF-8 character support (including Hungarian accented characters)
    SDL_StartTextInput();
    
    memset(currentWord, 0, sizeof(currentWord));
    memset(currentTry, 0, sizeof(currentTry));
    currentTime = 0;
    currentHead = NULL;
    renderScreen();
}

void endScreen()
{
    if (font) TTF_CloseFont(font);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    TTF_Quit();
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
    
    // Handle SDL_TEXTINPUT for UTF-8 characters and regular ASCII (including Hungarian accented characters)
    if (event->type == SDL_TEXTINPUT)
    {
        const char *text = event->text.text;
        // Process each byte of the UTF-8 string through keyPressed
        // This handles both ASCII and UTF-8 multi-byte characters
        for (int i = 0; text[i] != '\0'; i++)
        {
            keyPressed((unsigned char)text[i]);
        }
        return;  // Don't process this as KEYDOWN as well
    }
    
    // Handle SDL_KEYDOWN only for special keys (not text characters)
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
    // DO NOT handle a-z or 1-7 here - let SDL_TEXTINPUT handle those
    // Only special keys should be handled by KEYDOWN
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
