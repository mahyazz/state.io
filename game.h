
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

#define map_size 5
#define num_maps 5
#define max_players 5
#define max_soldiers 30
#define our_player 2

#define EVENT_QUIT 1
#define EVENT_TERMINATE 2
#define EVENT_CONTINUE 3
#define EVENT_MENU 4

#define INACTIVE 0

#define BG_COLOR 0xfffaf9f8

extern int board[map_size][map_size];
extern int soldiers[map_size][map_size];

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Surface *surface;
SDL_Texture *texture;
SDL_Surface *image;
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

// ---------------------------- Potion -----------------------------

typedef struct {
    int type;
    int owner;
    int timer;
    int x;
    int y;
    int state;
} Potion;

extern Potion potion;

#define STATE_ACTIVE 1
#define STATE_INACTIVE 2
#define STATE_TAKEN 3

#define TYPE_FREEZE 1

// ---------------------------- Functions -----------------------------

void manage_potion_cross(int k);