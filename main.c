#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "soldier_list.c"

int grid_width = 5;
int grid_height = 5;
int window_width = 600 + 1;
int window_height = 600 + 1;
const int FPS = 60;

Uint32 colors[][2] = {
    {0xfffaf9f8, 0xfffaf9f8},  // background - off-white
    {0xffe6e2de, 0xffbdb5ad},  // free - grey
    {0xffa1dfc0, 0xff90c49f},  // player 1 - green
    {0xfff4e8ad, 0xffecda78},  // player 2 - blue
    {0xabababff, 0x696969ff},  // player 3 - pink
    {0xff99eeff, 0xff4ce1ff},  // player 4 - yellow
    {0xffe5b7d2, 0xffe09ec1},  // player 4 - purple
};

int map[3][5][5] = {{
                        {0, 0, 1, 2, 0},
                        {0, 1, 1, 1, 0},
                        {1, 3, 1, 1, 0},
                        {3, 1, 1, 0, 0},
                        {0, 0, 4, 1, 0},
                    },
                    {
                        {0, 2, 1, 1, 0},
                        {0, 1, 1, 0, 0},
                        {1, 1, 3, 0, 1},
                        {4, 1, 1, 1, 5},
                        {1, 1, 0, 1, 0},
                    },
                    {
                        {0, 2, 1, 1, 1},
                        {0, 0, 1, 1, 1},
                        {0, 0, 0, 0, 0},
                        {1, 1, 1, 0, 0},
                        {1, 1, 1, 3, 0},
                    }};

int board[5][5];
int soldiers[5][5];
SDL_Renderer *renderer;

// ---------------------------- Map -----------------------------

void set_board(int k) {
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            board[i][j] = map[k][i][j];
        }
    }
}

void fill_cell(int x, int y, Uint32 color, Uint32 scolor) {
    x *= cell_size;
    y *= cell_size;
    boxColor(renderer, x, y, x + cell_size, y + cell_size, color);
    rectangleColor(renderer, x, y, x + cell_size, y + cell_size, scolor);
    // filledCircleColor(renderer, x + (cell_size / 2), y + (cell_size / 2),
    //                   cell_size / 6, scolor);
}

void draw_map() {
    // background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // land
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int type = board[i][j];
            fill_cell(j, i, colors[type][0], colors[type][1]);
        }
    }
}

// ---------------------------- Soldiers -----------------------------

void init_soldiers() {
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            for (int t = 0; t < 15; t++) {
                if (board[i][j] > 0) {
                    add_soldier(board[i][j], i, j);
                }
            }
        }
    }
}

void show_soldiers() {
    for (int k = 0; k < listsize; k++) {
        Soldier *sol = &list[k];
        Uint32 color = colors[sol->owner][1];
        int rsoldier = 8;
        if (sol->owner) {
            filledCircleColor(renderer, sol->x, sol->y, rsoldier, color);
        }
    }
}

void move_soldiers() {
    for (int k = 0; k < listsize; k++) {
        Soldier *sol = &list[k];
        if (sol->owner && (sol->dx || sol->dy)) {
            move_soldier(k);
        }
    }
}

// ---------------------------- Main -----------------------------

int handle_events() {
    // return 1 if must exit
    SDL_Event sdlEvent;
    static int x1, x2, y1, y2;
    while (SDL_PollEvent(&sdlEvent)) {
        switch (sdlEvent.type) {
            case SDL_QUIT:
                return 1;
                break;
            case SDL_MOUSEBUTTONDOWN:
                SDL_GetMouseState(&x1, &y1);
                printf("%d:%d\n", x1, y1);
                fflush(stdout);
            case SDL_MOUSEBUTTONUP:
                SDL_GetMouseState(&x2, &y2);
                printf("%d:%d\n", x2, y2);
                fflush(stdout);
                int i = floor(y1 / cell_size);
                int j = floor(x1 / cell_size);
                int i2 = floor(y2 / cell_size);
                int j2 = floor(x2 / cell_size);
                int delay = 0;
                for (int k = 0; k < listsize; k++) {
                    Soldier *sol = &list[k];
                    if (sol->i == i && sol->j == j) {
                        set_soldier_target(k, i2, j2, delay);
                        delay += 10;
                    }
                }
        }
    }
    return 0;
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

    if (SDL_CreateWindowAndRenderer(window_width, window_height, 0, &window,
                                    &renderer) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Create window and renderer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_SetWindowTitle(window, "state.io");

    // draw map
    set_board(0);
    init_soldiers();

    while (1) {
        if (handle_events()) {
            break;
        }

        draw_map();

        move_soldiers();
        show_soldiers();

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / FPS);
    }

    // cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
