#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int cell_size = 120;
int grid_width = 5;
int grid_height = 5;
int window_width = 600 + 1;
int window_height = 600 + 1;

void draw_background(SDL_Renderer *renderer, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer);
}

void draw_grids(SDL_Renderer *renderer, int window_width, int window_height,
                SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    for (int x = 0; x < 1 + grid_width * cell_size; x += cell_size) {
        SDL_RenderDrawLine(renderer, x, 0, x, window_height);
    }

    for (int y = 0; y < 1 + grid_height * cell_size; y += cell_size) {
        SDL_RenderDrawLine(renderer, 0, y, window_width, y);
    }
}

void fill_cell(SDL_Renderer *renderer, int x, int y, SDL_Color color,
               Uint32 scolor) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    SDL_Rect cell;
    cell.x = x;
    cell.y = y;
    cell.w = cell_size;
    cell.h = cell_size;

    SDL_RenderFillRect(renderer, &cell);

    filledCircleColor(renderer, x + 60, y + 60, 20, scolor);
}

void draw_map(SDL_Renderer *renderer) {
    // colors
    SDL_Color background = {233, 233, 233, 255};       // off-white
    SDL_Color grid_line_color = {200, 200, 200, 255};  // gray
    SDL_Color free_land_color = {108, 122, 137, 1};    // blue grey
    SDL_Color player_a_color = {217, 30, 24, 1};       // red
    SDL_Color player_b_color = {38, 166, 91, 1};       // green
    SDL_Color player_c_color = {0, 0, 255, 255};       // blue

    Uint32 free_land_scolor = 0xFF1E1800;  // black
    Uint32 player_a_scolor = 0xFF00008B;   // dark red
    Uint32 player_b_scolor = 0xFF008000;   // dark green
    Uint32 player_c_scolor = 0xFF800000;   // dark blue

    // background
    draw_background(renderer, background);

    // player land
    fill_cell(renderer, 0 * cell_size, 0 * cell_size, player_a_color,
              player_a_scolor);
    fill_cell(renderer, 2 * cell_size, 2 * cell_size, player_b_color,
              player_b_scolor);
    fill_cell(renderer, 1 * cell_size, 4 * cell_size, player_c_color,
              player_c_scolor);

    // free land
    fill_cell(renderer, 1 * cell_size, 0 * cell_size, free_land_color,
              free_land_scolor);
    fill_cell(renderer, 0 * cell_size, 1 * cell_size, free_land_color,
              free_land_scolor);
    fill_cell(renderer, 1 * cell_size, 1 * cell_size, free_land_color,
              free_land_scolor);
    fill_cell(renderer, 2 * cell_size, 1 * cell_size, free_land_color,
              free_land_scolor);
    fill_cell(renderer, 0 * cell_size, 2 * cell_size, free_land_color,
              free_land_scolor);
    fill_cell(renderer, 1 * cell_size, 2 * cell_size, free_land_color,
              free_land_scolor);
    fill_cell(renderer, 4 * cell_size, 2 * cell_size, free_land_color,
              free_land_scolor);
    fill_cell(renderer, 0 * cell_size, 3 * cell_size, free_land_color,
              free_land_scolor);
    fill_cell(renderer, 1 * cell_size, 3 * cell_size, free_land_color,
              free_land_scolor);
    fill_cell(renderer, 4 * cell_size, 3 * cell_size, free_land_color,
              free_land_scolor);

    // grid
    draw_grids(renderer, window_width, window_height, grid_line_color);

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
    SDL_bool quit = SDL_FALSE;

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = SDL_TRUE;
                    break;
            }
        }
    }

    // quitting
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}