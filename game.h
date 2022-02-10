
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

#define map_size 5
#define num_maps 5
#define max_players 5
#define max_soldiers 30
#define our_player 2

#define EVENT_QUIT 1
#define EVENT_BACK 2
#define EVENT_MENU 3

#define BG_COLOR 0xfffaf9f8

extern int board[map_size][map_size];
extern int soldiers[map_size][map_size];

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Surface *surface;
SDL_Texture *texture;
SDL_Surface *image;
SDL_Surface *potion;
TTF_Font *font;

// ---------------------------- Graphics -----------------------------

#define window_width 600
#define window_height 600
#define bar_height 30

#define FPS 90
#define steps 100
#define cell_size 120
#define sol_radius 7
#define eps 10
#define speed 2
#define leave_delay 12

// ---------------------------- Functions -----------------------------

void draw_bar();
