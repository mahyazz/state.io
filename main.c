#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "soldier_list.c"

int bar_height = 30;
int map_size = 5;
int window_width = 600 + 1;
int max_players = 5;
int max_soldiers = 30;
int board[5][5];
int soldiers[5][5];
int score = 0;
int current_level = 0;

SDL_Renderer *renderer;
SDL_Surface *surface;
SDL_Texture *texture;
SDL_Surface *image;
SDL_Surface *potion;
TTF_Font *font;

Uint32 colors[][2] = {
    {0xfffaf9f8, 0xfffaf9f8},  // background - off-white
    {0xffe6e2de, 0xffbdb5ad},  // free - grey
    {0xffa1dfc0, 0xff90c49f},  // player 2 - green
    {0xfff4e8ad, 0xffecda78},  // player 3 - blue
    {0xffc6c1ff, 0xffa39bfe},  // player 4 - pink
    {0xff99eeff, 0xff4ce1ff},  // player 5 - yellow
    {0xffe5b7d2, 0xffe09ec1},  // potion - purple
};

SDL_Color color_black = {0, 0, 0};
SDL_Color color_white = {255, 255, 255};

int map[][5][5] = {{
                       {0, 0, 2, 0, 0},
                       {0, 0, 2, 0, 0},
                       {0, 0, 1, 3, 0},
                       {0, 0, 2, 0, 0},
                       {0, 0, 2, 0, 0},
                   },
                   {
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

// ---------------------------- Font -----------------------------

void init_font() {
    TTF_Init();
    font = TTF_OpenFont("FreeSans.ttf", 24);
    if (font == NULL) {
        fprintf(stderr, "error: font not found\n");
        exit(EXIT_FAILURE);
    }
}

void put_text(int x, int y, char *text, SDL_Color color) {
    int text_width;
    int text_height;
    SDL_Rect rect;
    // SDL_Color textColor = {0, 0, 0, 200};

    surface = TTF_RenderText_Blended(font, text, color);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    text_width = surface->w;
    text_height = surface->h;
    SDL_FreeSurface(surface);

    rect.x = x - text_width / 2;
    rect.y = y - text_height / 2;
    rect.w = text_width;
    rect.h = text_height;
    SDL_RenderCopy(renderer, texture, NULL, &rect);
}

// ---------------------------- Panel -----------------------------

void get_click(int *x, int *y) {
    SDL_Event event;
    while (true) {
        SDL_PollEvent(&event);
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            SDL_GetMouseState(x, y);
            return;
        }
    }
}

bool confirm(char *message) {
    boxColor(renderer, 0, 0, window_width, window_width, 0x66000000);

    boxColor(renderer, 150, 200, 450, 400, 0xffcccccc);
    boxColor(renderer, 150, 350, 300, 400, 0xff3845b4);
    boxColor(renderer, 300, 350, 450, 400, 0xff7db739);

    put_text(300, 260, message, color_black);
    put_text(225, 375, "No", color_white);
    put_text(375, 375, "Yes", color_white);
    SDL_RenderPresent(renderer);
    int x, y;
    while (true) {
        get_click(&x, &y);
        if (150 < x && x < 300 && 350 < y && y < 400) return false;
        if (300 < x && x < 450 && 350 < y && y < 400) return true;
    }
}

// ---------------------------- Score -----------------------------

int find_winner() {
    bool all_the_same = true;
    int winner = 0;
    for (int i = 0; i < map_size; i++) {
        for (int j = 0; j < map_size; j++) {
            int owner = board[i][j];
            if (owner >= 2) {
                if (!winner) {
                    winner = owner;
                } else {
                    all_the_same = all_the_same && winner == owner;
                }
            }
        }
    }
    return all_the_same ? winner : 0;
}

void determine_score(int winner) {
    if (winner == 2) {
        score += 100;
    } else {
        score -= 50;
    }
}

void show_result(int winner) {
    if (winner == 2) {
        image = SDL_LoadBMP("winimage.bmp");
    } else {
        image = SDL_LoadBMP("loseimage.bmp");
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, image);
    SDL_Rect dstrect = {0, 0, window_width, window_width};
    SDL_RenderCopy(renderer, texture, NULL, &dstrect);
}

// ---------------------------- Navigation Bar -----------------------------

void draw_bar() {
    hlineColor(renderer, 0, window_width, window_width, 0x88000000);
    boxColor(renderer, 0, window_width - 1, window_width,
             window_width + bar_height - 1, colors[1][1]);
    char *str1 = "Menu";
    char *str2 = "Score:";
    stringColor(renderer, 10, window_width + 13, str1, 0x88000000);
    stringColor(renderer, window_width - 90, window_width + 13, str2,
                0x88000000);
    char str[10];
    sprintf(str, "%d", score);
    stringColor(renderer, window_width - 40, window_width + 13, str,
                0x88000000);
}

// ---------------------------- Map -----------------------------

void set_board(int k) {
    for (int i = 0; i < map_size; i++) {
        for (int j = 0; j < map_size; j++) {
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
    for (int i = 0; i < map_size; i++) {
        for (int j = 0; j < map_size; j++) {
            int type = board[i][j];
            fill_cell(i, j, colors[type][0], colors[type][1]);
        }
    }
}

void update_board() {
    for (int i = 0; i < map_size; i++) {
        for (int j = 0; j < map_size; j++) {
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
    int bradius = cell_size / 6;
    Uint32 color = colors[board[i][j]][1] & 0x55ffffff;
    filledCircleColor(renderer, i + center, j + center, bradius, color);
}

// ---------------------------- Soldiers -----------------------------

void init_soldiers() {
    for (int i = 0; i < map_size; i++) {
        for (int j = 0; j < map_size; j++) {
            for (int t = 0; t < 15; t++) {
                if (board[i][j] > 0) {
                    add_soldier(board[i][j], i, j);
                }
            }
        }
    }
}

void generate_soldiers() {
    for (int i = 0; i < map_size; i++) {
        for (int j = 0; j < map_size; j++) {
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
        if (sol->owner && sol->i == i && sol->j == j && !is_moving(k)) {
            set_soldier_target(k, i2, j2, delay);
            delay += leave_delay;
        }
    }
}

int is_better(i, j, i1, j1, i2, j2) {
    // check whether (i1, j1) is better than (i2, j2) for (i, j)
    int dist1 = abs(i - i1) + abs(j - j1);
    int dist2 = abs(i - i2) + abs(j - j2);
    return (dist1 < dist2) || soldiers[i1][j1] < soldiers[i2][j2];
}

void best_target(int i, int j) {
    int best_i = 100, best_j = 0;
    for (int x = 0; x < map_size; x++) {
        for (int y = 0; y < map_size; y++) {
            if (soldiers[i][j] > soldiers[x][y] && board[x][y] &&
                board[x][y] != board[i][j]) {
                if (is_better(i, j, x, y, best_i, best_j)) {
                    best_i = x;
                    best_j = y;
                }
            }
        }
    }
    if (best_i < 100) {
        move_to_target(i, j, best_i, best_j);
    }
}

void random_attacker() {
    if (rand() % (rand() % (3 * FPS))) return;
    int i = rand() % map_size;
    int j = rand() % map_size;
    if (board[i][j] > 2) {
        best_target(i, j);
    }
}

void send_help(int x, int y) {
    for (int i = 0; i < map_size; i++) {
        for (int j = 0; j < map_size; j++) {
            if (board[i][j] == board[x][y] && x != i && y != j) {
                move_to_target(i, j, x, y);
                return;
            }
        }
    }
}

void defend() {
    for (int i = 0; i < map_size; i++) {
        for (int j = 0; j < map_size; j++) {
            for (int t = 0; t < listsize; t++) {
                if (list[t].i == i && list[t].j == j &&
                    list[t].owner != board[i][j]) {
                    send_help(i, j);
                }
            }
        }
    }
}

// ---------------------------- Potions -----------------------------

void draw_potion(int i, int j) {
    SDL_Surface *potion;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, potion);
    SDL_Rect dstrect = {i - 10, j - 10, i + 10, j + 10};
    SDL_RenderCopy(renderer, texture, NULL, &dstrect);
}

void place_potion() {
    int x = rand() % map_size;
    int y = rand() % map_size;

    int checki = 0, checkj = 0;
    for (int i = 0; i < x; i++) {
        checki = board[i][y] ? 1 : 0;
    }
    for (int i = x; i < map_size; i++) {
        checki = board[i][y] ? 1 : 0;
    }
    for (int j = 0; j < y; j++) {
        checkj = board[x][j] ? 1 : 0;
    }
    for (int j = x; j < map_size; j++) {
        checkj = board[x][j] ? 1 : 0;
    }

    if (checki || checkj) {
        draw_potion(x, y);
    } else {
        place_potion();
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

void init_game(int level) {
    set_board(level);
    listsize = 0;
    init_soldiers();
}

int main() {
    // gfxPrimitivesSetFont(Font18, 18, 18);

    // init
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Initialize SDL: %s",
                     SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window *window;
    if (SDL_CreateWindowAndRenderer(window_width, window_width + bar_height, 0,
                                    &window, &renderer) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Create window and renderer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_SetWindowTitle(window, "state.io");

    init_font();
    init_game(current_level);
    srand(time(0));

    // game loop
    int tik = 0;
    bool quit = false;

    while (true) {
        while (true) {
            if (handle_events()) {
                quit = true;
                break;
            }

            update_board();
            if (tik++ % (FPS / 1) == 0) {
                generate_soldiers();
            }
            draw_map();
            draw_bar();
            random_attacker();
            move_soldiers();
            // defend();
            // place_potion();
            show_soldiers();

            SDL_RenderPresent(renderer);
            SDL_Delay(1000 / FPS);

            int winner = find_winner();
            if (winner) {
                // determine_score(winner);
                draw_bar();
                break;
            }
        }
        if (quit) break;
        if (confirm("You win! Continue?")) {
            printf("Confirmed!\n");
            fflush(stdout);
            init_game(current_level + 1);
        } else {
            break;
        }
    }

    // cleanup
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(image);
    SDL_FreeSurface(potion);
    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}
