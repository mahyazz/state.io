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

Uint32 colors[][2] = {
    {0xfffaf9f8, 0xfffaf9f8},  // background - off-white
    {0xffe6e2de, 0xffbdb5ad},  // free - grey
    {0xffa1dfc0, 0xff90c49f},  // player 2 - green
    {0xfff4e8ad, 0xffecda78},  // player 3 - blue
    {0xffc6c1ff, 0xffa39bfe},  // player 4 - pink
    {0xff99eeff, 0xff4ce1ff},  // player 5 - yellow
    {0xffe5b7d2, 0xffe09ec1},  // potion - purple
};

int max_players = 5;
int max_soldiers = 30;
int map_size = 5;

int map[3][5][5] = {{
                        {0, 0, 2, 3, 0},
                        {0, 1, 1, 1, 0},
                        {4, 1, 1, 1, 0},
                        {3, 1, 1, 0, 0},
                        {0, 2, 1, 4, 0},
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
SDL_Surface *surface;

// ---------------------------- Map -----------------------------

void set_board(int k) {
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            board[i][j] = map[k][i][j];
        }
    }
}

void fill_cell(int i, int j, Uint32 color, Uint32 ccolor) {
    int x = j * cell_size;
    int y = i * cell_size;
    boxColor(renderer, x, y, x + cell_size, y + cell_size, color);
    rectangleColor(renderer, x, y, x + cell_size, y + cell_size, ccolor);
    filledCircleColor(renderer, x + (cell_size / 2), y + (cell_size / 2),
                      cell_size / 7, ccolor & 0x55ffffff);
    if (board[i][j]) {
        char str[10];
        sprintf(str, "%d", soldiers[i][j]);
        stringColor(renderer, x + (cell_size / 2) - strlen(str) / 2 * 8,
                    y + (cell_size / 2) + 2 * sol_radius, str, 0x88000000);
    }
}

void draw_map() {
    // background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // land
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int type = board[i][j];
            fill_cell(i, j, colors[type][0], colors[type][1]);
        }
    }
}

void update_board() {
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            soldiers[i][j] = 0;
            if (board[i][j]) {
                for (int k = 0; k < listsize; k++) {
                    if (is_inside(k, i, j, 1) && !is_moving(k) &&
                        list[k].owner) {
                        soldiers[i][j]++;
                        board[i][j] = list[k].owner;
                    }
                }
            }
        }
    }
}

void trench_animation(int i, int j) {
    int center = cell_size / 2;
    int sradius = cell_size / 7, bradius = cell_size / 6;
    int type = board[i][j];
    filledCircleColor(renderer, i + center, j + center, bradius,
                      colors[type][1]);
    filledCircleColor(renderer, i + center, j + center, sradius,
                      colors[type][1]);
    filledCircleColor(renderer, i + center, j + center, bradius,
                      colors[type][1]);
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

void generate_soldiers() {
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (board[i][j] > 1 && soldiers[i][j] < max_soldiers) {
                add_soldier(board[i][j], i, j);
                soldiers[i][j]++;
            }
        }
    }
}

void show_soldiers() {
    for (int k = 0; k < listsize; k++) {
        Soldier *sol = &list[k];
        Uint32 color = colors[sol->owner][1];
        if (sol->owner) {
            filledCircleColor(renderer, sol->x, sol->y, sol_radius, color);
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

void move_to_target(int i, int j, int i2, int j2) {
    printf("%d:%d -> %d:%d\n", i, j, i2, j2);
    fflush(stdout);
    if (!board[i2][j2]) return;
    if (board[i][j] < 2) return;
    int delay = 0;
    for (int k = 0; k < listsize; k++) {
        Soldier *sol = &list[k];
        if (sol->i == i && sol->j == j && !is_moving(k)) {
            set_soldier_target(k, i2, j2, delay);
            delay += leave_delay;
        }
    }
}

void random_move() {
    if (rand() % (rand() % (3 * FPS))) return;
    int i = rand() % map_size;
    int j = rand() % map_size;
    int i2 = rand() % map_size;
    int j2 = rand() % map_size;
    if (board[i][j] > 2) {
        move_to_target(i, j, i2, j2);
    }
}

// ---------------------------- Potions -----------------------------

// void drop_potion() {
//     do{
//      int x = rand() % map_size;
//     int y = rand() % map_size;
//     } while (board[x][y] )
// }

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
                int i = floor(y1 / cell_size);
                int j = floor(x1 / cell_size);
                printf("soldiers[%d][%d]: %d\n", i, j, soldiers[i][j]);
                // printf("%d:%d\n", x1, y1);
                fflush(stdout);
                break;
            case SDL_MOUSEBUTTONUP:
                SDL_GetMouseState(&x2, &y2);
                // printf("%d:%d\n", x2, y2);
                // fflush(stdout);
                i = floor(y1 / cell_size);
                j = floor(x1 / cell_size);
                int i2 = floor(y2 / cell_size);
                int j2 = floor(x2 / cell_size);
                if (board[i][j] == 2) {
                    move_to_target(i, j, i2, j2);
                }
                break;
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
    SDL_Texture *texture = NULL;

    if (SDL_CreateWindowAndRenderer(window_width, window_width, 0, &window,
                                    &renderer) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Create window and renderer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_SetWindowTitle(window, "state.io");

    // draw map
    set_board(0);
    init_soldiers();

    printf("%d %f\n", listsize, list[0].x);

    int tik = 0;
    while (1) {
        if (handle_events()) {
            break;
        }

        update_board();
        if (tik++ % (FPS / 1) == 0) {
            generate_soldiers();
        }
        draw_map();
        random_move();
        move_soldiers();
        show_soldiers();

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / FPS);
    }

    // cleanup
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    SDL_Quit();

    return EXIT_SUCCESS;
}
