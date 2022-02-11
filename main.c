#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "soldiers.h"

// ---------------------------- Globals -----------------------------

int current_level = 0;
int num_players[num_maps];
int board[map_size][map_size];
int soldiers[map_size][map_size];
int map[num_maps][map_size][map_size];
int score[7] = {-1, -1, 0, 0, 0, 0, 0};
char players[][25] = {"You", "Blue Pal", "Pink Pal", "Yellow Pal",
                      "Purple Pal"};
char potions[][25] = {"Freeze", "No Bound", "High Speed", "Low Speed",
                      "No Attack"};
Potion potion;

// ---------------------------- Colors -----------------------------

Uint32 colors[][2] = {
    {BG_COLOR, BG_COLOR},      // background - off-white
    {0xffe6e2de, 0xffbdb5ad},  // free - grey
    {0xffa1dfc0, 0xff90c49f},  // player 2 - green
    {0xfff4e8ad, 0xffecda78},  // player 3 - blue
    {0xffc6c1ff, 0xffa39bfe},  // player 4 - pink
    {0xff99eeff, 0xff4ce1ff},  // player 5 - yellow
    {0xffe5b7d2, 0xffe09ec1},  // player 6 - purple
};

SDL_Color color_black = {0, 0, 0};
SDL_Color color_white = {255, 255, 255};

// --------------------------- Local Headers ---------------------------

void draw_bar();

// --------------------------- Handy Functions --------------------------

int get_click(int *x, int *y) {
    SDL_Event event;
    while (true) {
        SDL_PollEvent(&event);
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            SDL_GetMouseState(x, y);
            return 1;
        }
    }
}

bool inside_box(int x, int y, int x1, int x2, int y1, int y2) {
    return (x1 <= x && x <= x2 && y1 <= y && y <= y2);
}

// ---------------------------- Font -----------------------------

void init_font() {
    TTF_Init();
    font = TTF_OpenFont("resources/FreeSans.ttf", 24);
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
    SDL_DestroyTexture(texture);
}

// ---------------------------- Score -----------------------------

int find_winner() {
    int count[max_players + 2];
    for (int i = 0; i < max_players + 2; i++) {
        count[i] = 0;
    }
    for (int i = 0; i < map_size; i++) {
        for (int j = 0; j < map_size; j++) {
            count[board[i][j]]++;
        }
    }
    int max = 2;
    for (int i = 2; i < max_players + 2; i++) {
        if (count[i] > count[max]) max = i;
    }
    int single_winner = true;
    for (int i = 3; i < max_players + 2; i++) {
        single_winner = single_winner && count[i] == 0;
    }
    if (count[our_player] == 0 || max == our_player && single_winner) {
        return max;
    }
    return 0;
}

void determine_score(int winner) {
    for (int i = 2; i < num_players[current_level] + 2; i++) {
        if (i != winner) {
            score[i] = fmax(score[i] - 50, 0);
        }
    }
    score[winner] += 100;
}

// ---------------------------- Save & Load -----------------------------

void save_scores() {
    FILE *file = fopen("data/scores.txt", "w");
    for (int i = 0; i < max_players; i++) {
        fprintf(file, "%d \n", score[i + 2]);
    }
    fclose(file);
}

void load_scores() {
    FILE *file = fopen("data/scores.txt", "r");
    if (file == NULL) {
        return;
    }
    for (int i = 0; i < max_players; i++) {
        fscanf(file, "%d ", &score[i + 2]);
    }
    fclose(file);
}

void save_game() {
    FILE *file = fopen("data/game.dat", "w");
    // save potion
    fprintf(file, "%d %d %d %d %d %d\n", potion.type, potion.owner,
            potion.timer, potion.x, potion.y, potion.state);

    // save board
    fprintf(file, "%d\n", current_level);
    fprintf(file, "%d\n", num_players[current_level]);
    for (int i = 0; i < map_size; i++) {
        for (int j = 0; j < map_size; j++) {
            fprintf(file, "%d ", board[i][j]);
        }
        fprintf(file, "\n");
    }

    // save soldiers
    fprintf(file, "%d\n", listsize);
    for (int i = 0; i < listsize; i++) {
        Soldier s = list[i];
        fprintf(file, "%d %.0f %.0f %.2f %.2f %d %d %d", s.owner, s.x, s.y,
                s.dx, s.dy, s.i, s.j, s.delay);
        fprintf(file, "\n");
    }

    fclose(file);
}

void load_game() {
    FILE *file = fopen("data/game.dat", "r");
    if (file == NULL) {
        return;
    }

    // load potion
    fscanf(file, "%d %d %d %d %d %d", &potion.type, &potion.owner,
           &potion.timer, &potion.x, &potion.y, &potion.state);

    // load board
    fscanf(file, "%d", &current_level);
    fscanf(file, "%d", &num_players[current_level]);

    for (int i = 0; i < map_size; i++) {
        for (int j = 0; j < map_size; j++) {
            fscanf(file, "%d ", &board[i][j]);
        }
    }

    // load soldiers
    fscanf(file, "%d", &listsize);
    for (int i = 0; i < listsize; i++) {
        Soldier *s = list + i;
        fscanf(file, "%d %f %f %f %f %d %d %d", &s->owner, &s->x, &s->y, &s->dx,
               &s->dy, &s->i, &s->j, &s->delay);
    }

    // load players
    fclose(file);
}

void load_maps() {
    FILE *file = fopen("data/maps.txt", "r");
    if (file == NULL) {
        return;
    }

    for (int k = 0; k < num_maps; k++) {
        fscanf(file, "%d ", &num_players[k]);
        for (int i = 0; i < map_size; i++) {
            for (int j = 0; j < map_size; j++) {
                fscanf(file, "%d ", &map[k][i][j]);
            }
        }
    }
    fclose(file);
}

// -------------------------- Init Game ---------------------------

void set_board(int k) {
    for (int i = 0; i < map_size; i++) {
        for (int j = 0; j < map_size; j++) {
            board[i][j] = map[k][i][j];
        }
    }
}

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

void init_game(int level) {
    current_level = level;
    set_board(level);
    listsize = 0;
    init_soldiers();
    potion.state = STATE_INACTIVE;
}

void create_random_map() {
    int level = map_size - 1;
    num_players[level] = 2 + rand() % (max_players - 1);
    for (int i = 0; i < map_size; i++) {
        for (int j = 0; j < map_size; j++) {
            int type = rand() % (num_players[level] + 2);
            map[num_maps - 1][i][j] = (type == 0) ? 1 : type;
        }
    }
    for (int k = 0; k < map_size * map_size / 2; k++) {
        int i = rand() % map_size;
        int j = rand() % map_size;
        if (i % (level) && j % (level)) continue;
        map[level][i][j] = 0;
    }
    init_game(level);
    if (find_winner()) {
        create_random_map();
    }
}

// ------------------------- Sort Scoreboard --------------------------

typedef struct {
    int id;
    int score;
} Player;

void swap(Player *p, Player *q) {
    Player temp = *p;
    *p = *q;
    *q = temp;
}

void sort(Player *A, int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = n - 1; j > i; j--) {
            if (A[j].score > A[j - 1].score) {
                swap(A + j, A + j - 1);
            }
        }
    }
}

// --------------------------- Menu & Scoreboard -----------------------------

int get_option(char options[][30], int n) {
    boxColor(renderer, 0, 0, window_width, window_height, BG_COLOR);
    int height = 600 / (n + 2);
    float box_ratio = 3.0 / 4;
    int x, y;
    for (int i = 1; i <= n; i++) {
        boxColor(renderer, 150, height * i, 450, height * (i + box_ratio),
                 0xffe6e2de);
        put_text(300, height * (i + 1.0 / 3), options[i - 1], color_black);
    }
    draw_bar();
    SDL_RenderPresent(renderer);
    while (true) {
        get_click(&x, &y);
        if (150 <= x && x <= 450) {
            int k = floor(y / height);
            if (1 <= k && k <= n && y <= height * (k + box_ratio)) {
                return k;
            }
        }
    }
}

void show_scoreboard() {
    Player scores[max_players];
    for (int i = 0; i < max_players; i++) {
        scores[i].id = i;
        scores[i].score = score[i + 2];
    }
    sort(scores, max_players);
    char options[max_players + 2][30];
    strcpy(options[0], "SCOREBOARD");
    for (int i = 0; i < max_players; i++) {
        sprintf(options[i + 1], "%s: %d", players[scores[i].id],
                scores[i].score);
    }
    strcpy(options[max_players + 1], "Back");
    get_option(options, max_players + 2);
}

char options[][30] = {"Last Saved Game", "Map 1",      "Map 2",      "Map 3",
                      "Map 4",           "Random Map", "Scoreboard", "Quit"};

int show_menu(int back) {
    int result = 0;
    strcpy(options[7], back ? "Back" : "Quit");
    int k = get_option(options, 8);
    if (k == 1) {
        load_game();
    } else if (k == 6) {
        create_random_map();
    } else if (k == 7) {
        show_scoreboard();
        result = show_menu(back);
    } else if (k == 8) {
        return EVENT_QUIT;
    } else {
        current_level = k - 2;
        init_game(current_level);
    }
    return result;
}

// ---------------------------- Panel -----------------------------

bool confirm(char *message) {
    boxColor(renderer, 0, 0, window_width, window_height, 0xff666666);

    boxColor(renderer, 150, 200, 450, 400, 0xffcccccc);
    boxColor(renderer, 150, 350, 300, 400, 0xff3845b4);
    boxColor(renderer, 300, 350, 450, 400, 0xff7db739);

    put_text(300, 260, message, color_black);
    put_text(225, 375, "No", color_white);
    put_text(375, 375, "Yes", color_white);

    draw_bar();
    SDL_RenderPresent(renderer);

    int x, y;
    while (true) {
        get_click(&x, &y);
        if (inside_box(x, y, 150, 300, 350, 400)) return false;
        if (inside_box(x, y, 300, 450, 350, 400)) return true;
    }
}

// ---------------------------- Navigation Bar -----------------------------

void draw_bar() {
    char str[100];
    int gray = 0x88000000;
    hlineColor(renderer, 0, window_width, window_height, gray);
    boxColor(renderer, 0, window_width - 1, window_height,
             window_height + bar_height - 1, colors[1][1]);
    stringColor(renderer, 12, window_height + 12, "Menu", gray);

    sprintf(str, "Level: %d   Score: %d", current_level + 1, score[our_player]);
    stringColor(renderer, window_width - strlen(str) * 8 - 12,
                window_height + 12, str, gray);

    // printf("type: %d\n", potion.type);
    // fflush(stdout);

    if (potion.state == STATE_TAKEN) {
        float limit = 5 * FPS;
        boxColor(renderer, 240, 604, 360, 624, colors[potion.owner][1]);
        boxColor(renderer, 240 + (limit - potion.timer) / limit * 120, 604, 360,
                 624, colors[potion.owner][0]);
    }

    if (potion.state != STATE_INACTIVE) {
        char *s = (char *)potions[potion.type - 1];
        stringColor(renderer, window_width / 2 - strlen(s) / 2 * 8,
                    window_height + 12, s, gray);
    }
}

// ---------------------------- Map -----------------------------

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

// ---------------------------- Soldiers -----------------------------

void generate_soldiers() {
    for (int i = 0; i < map_size; i++) {
        for (int j = 0; j < map_size; j++) {
            if (board[i][j] > 1 &&
                (soldiers[i][j] < max_soldiers ||
                 player_has_potion(board[i][j], TYPE_NO_BOUND))) {
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
        Soldier sol = list[k];
        if (sol.owner && is_moving(k)) {
            move_soldier(k);
        }
    }
}

void move_to_target(int i, int j, int i2, int j2) {
    // printf("%d:%d -> %d:%d\n", i, j, i2, j2);
    // fflush(stdout);
    if (!board[i2][j2]) return;
    if (board[i][j] < our_player) return;
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
            if (soldiers[i][j] > soldiers[x][y] + abs(i - x) + abs(j - y) &&
                board[x][y] && board[x][y] != board[i][j]) {
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
    if (board[i][j] > our_player) {
        best_target(i, j);
    }
}

// ---------------------------- Potions -----------------------------

bool player_has_potion(int player, int type) {
    return potion.state == STATE_TAKEN && potion.owner == player &&
           potion.type == type;
}

bool others_has_potion(int player, int type) {
    return potion.state == STATE_TAKEN && potion.owner != player &&
           potion.type == type;
}

void show_potion() {
    if (--potion.timer <= 0) {
        potion.state = STATE_INACTIVE;
    }
    if (potion.state != STATE_ACTIVE) return;
    image = SDL_LoadBMP("resources/potion.bmp");
    texture = SDL_CreateTextureFromSurface(renderer, image);
    SDL_Rect dstrect = {potion.x - 20, potion.y - 25, 40, 50};
    SDL_RenderCopy(renderer, texture, NULL, &dstrect);
    SDL_FreeSurface(image);
    SDL_DestroyTexture(texture);
}

float dist2(float x1, float y1, float x2, float y2) {
    return (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
}

bool is_accessible(int x, int y, int r) {
    for (int i = 0; i < map_size; i++) {
        for (int j = 0; j < map_size; j++) {
            for (int i2 = 0; i2 < map_size; i2++) {
                for (int j2 = 0; j2 < map_size; j2++) {
                    if (board[i][j] == 2 && board[i2][j2] >= 1) {
                        float x1 = (j + 0.5) * cell_size;
                        float y1 = (i + 0.5) * cell_size;
                        float x2 = (j2 + 0.5) * cell_size;
                        float y2 = (i2 + 0.5) * cell_size;
                        float iterations = 50;
                        float dx = (x2 - x1) / iterations,
                              dy = (y2 - y1) / iterations;
                        for (int k = 0; k < iterations; k++) {
                            if (dist2(x, y, x1, y1) <= r * r) {
                                return true;
                            }
                            x1 += dx;
                            y1 += dy;
                        }
                    }
                }
            }
        }
    }
    return false;
}

void place_potion() {
    while (potion.state == STATE_INACTIVE) {
        int x = 25 + rand() % (window_width - 50);
        int y = 25 + rand() % (window_height - 50);

        if (is_accessible(x, y, 7)) {
            potion.x = x;
            potion.y = y;
            potion.state = STATE_ACTIVE;
            potion.type = 1 + rand() % 5;
            potion.timer = 5 * FPS;
        }
    }
}

void manage_potion_cross(int k) {
    Soldier sol = list[k];
    if (potion.state == STATE_ACTIVE &&
        dist2(sol.x, sol.y, potion.x, potion.y) < 20 * 20) {
        potion.state = STATE_TAKEN;
        potion.owner = sol.owner;
        potion.timer = 5 * FPS;
    }
}

// ---------------------------- Main -----------------------------

int handle_events() {
    SDL_Event sdlEvent;
    static int x1, x2, y1, y2;
    static bool clicked = false;
    while (SDL_PollEvent(&sdlEvent)) {
        switch (sdlEvent.type) {
            case SDL_QUIT:
                return EVENT_QUIT;
                break;
            case SDL_MOUSEBUTTONDOWN:
                SDL_GetMouseState(&x1, &y1);
                if (inside_box(x1, y1, 0, 100, window_width,
                               window_height + bar_height)) {
                    show_menu(1);
                }
                clicked = true;
                break;
            case SDL_MOUSEBUTTONUP:
                SDL_GetMouseState(&x2, &y2);
                int i = floor(y1 / cell_size);
                int j = floor(x1 / cell_size);
                int i2 = floor(y2 / cell_size);
                int j2 = floor(x2 / cell_size);
                if (clicked && board[i][j] == our_player) {
                    move_to_target(i, j, i2, j2);
                }
                clicked = false;
                break;
        }
    }
    return 0;
}

void init_sdl() {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(window_width, window_height + bar_height, 0,
                                &window, &renderer);
    SDL_SetWindowTitle(window, "state.io");
    init_font();
}

void cleanup() {
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    TTF_Quit();
    SDL_Quit();
}

int main() {
    srand(time(0));

    // username
    printf("Please enter your username (press return to skip):\n");
    fflush(stdout);
    scanf("%[^\n]%*c", (char *)players[0]);
    if (strlen(players[0]) == 0) {
        strcpy(players[0], "You");
    }

    init_sdl();
    load_maps();
    create_random_map();
    load_scores();

    int tik = 0;
    int event = 0;
    int winner = 0;

    while (true) {
        if (event != EVENT_CONTINUE && show_menu(0) == EVENT_QUIT) {
            event = EVENT_TERMINATE;
            break;
        }

        while (true) {
            event = handle_events();
            if (event == EVENT_MENU) {
                show_menu(1);
            }
            if (event == EVENT_QUIT) break;

            update_board();
            if (tik % (FPS / 1) == 0) {
                generate_soldiers();
            }
            draw_map();
            draw_bar();
            random_attacker();
            move_soldiers();
            // defend();
            show_soldiers();

            if (rand() % (FPS * 5) == 0) {
                place_potion();
            }
            show_potion();

            SDL_RenderPresent(renderer);
            SDL_Delay(1000 / FPS);

            winner = find_winner();
            if (winner) {
                determine_score(winner);
                draw_bar();
                break;
            }
            tik++;
        }
        if (event == EVENT_QUIT) break;
        if (winner == our_player) {
            if (confirm("You win! Continue?")) {
                init_game((current_level + 1) % num_maps);
                event = EVENT_CONTINUE;
            }
        } else if (confirm("You lose! Replay?")) {
            init_game(current_level);
            event = EVENT_CONTINUE;
        }
    }

    save_scores();
    if (event != EVENT_TERMINATE && confirm("Save game?")) {
        save_game();
    }

    cleanup();
    return EXIT_SUCCESS;
}
