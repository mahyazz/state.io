#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int cell_size = 120;
int grid_width = 5;
int grid_height = 5;
int window_width = 600 + 1;
int window_height = 600 + 1;

Uint32 colors[][2] = {
    {0xfffaf9f8, 0xfffaf9f8},  // background - off-white
    {0xffe6e2de, 0xffbdb5ad},  // free - grey
    {0xffa1dfc0, 0xff90c49f},  // player 1 - green
    {0xfff4e8ad, 0xffecda78},  // player 2 - blue
    {0xabababff, 0x696969ff},  // player 3 - pink
    {0xff99eeff, 0xff4ce1ff},  // player 4 - yellow
    {0xffe5b7d2, 0xffe09ec1},  // player 4 - purple
};

int board[5][5] = {
    {0, 0, 1, 2, 0},  // row
    {0, 1, 1, 1, 0},  // row
    {1, 3, 1, 1, 0},  // row
    {0, 1, 1, 0, 0},  // row
    {0, 0, 4, 1, 0},  // row
};

void fill_cell(SDL_Renderer *renderer, int x, int y, Uint32 color,
               Uint32 scolor) {
    x *= cell_size;
    y *= cell_size;
    boxColor(renderer, x, y, x + cell_size, y + cell_size, color);
    rectangleColor(renderer, x, y, x + cell_size, y + cell_size, scolor);
    filledCircleColor(renderer, x + 60, y + 60, 20, scolor);
}

void draw_map(SDL_Renderer *renderer) {
    // background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // land
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int type = board[i][j];
            fill_cell(renderer, j, i, colors[type][0], colors[type][1]);
        }
    }

    // render
    SDL_RenderPresent(renderer);
}

int main() {
    // init
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Initialize SDL: %s",
                     SDL_GetError());
        return EXIT_FAILURE;
    }

    // create window and renderer
    SDL_Window *window;
    SDL_Renderer *renderer;

    if (SDL_CreateWindowAndRenderer(window_width, window_height, 0, &window,
                                    &renderer) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Create window and renderer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_SetWindowTitle(window, "state.io");

    // draw map
    draw_map(renderer);

    // event handling
    bool quit = false;
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
            }
        }
    }

    // cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}