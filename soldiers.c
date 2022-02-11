#include "soldiers.h"

#include "game.h"

Soldier list[2500];
int listsize = 0;

void set_locatoin(int k, int i, int j) {
    Soldier *sol = &list[k];
    sol->x = (j + 0.5) * cell_size;
    sol->y = (i + 0.5) * cell_size;
}

void init_soldier(int k, int owner, int i, int j) {
    Soldier *sol = &list[k];
    // printf("%d", owner);
    sol->owner = owner;
    sol->i = i;
    sol->j = j;
    set_locatoin(k, i, j);
    sol->dx = 0;
    sol->dy = 0;
    sol->delay = 0;
}

void add_soldier(int owner, int i, int j) {
    int found = 0;
    for (int k = 0; k < listsize && !found; k++) {
        if (list[k].owner == 0) {
            init_soldier(k, owner, i, j);
            found = 1;
        }
    }
    if (!found) {
        init_soldier(listsize, owner, i, j);
        listsize++;
    }
}

void remove_soldier(int k) { list[k].owner = 0; }

int is_moving(int k) { return list[k].dx || list[k].dy; }

int is_inside(int k, int i, int j, float r) {
    Soldier *sol = &list[k];
    return (fabs(sol->x - (j + 0.5) * cell_size) < r &&
            fabs(sol->y - (i + 0.5) * cell_size) < r);
}

void set_soldier_target(int k, int i, int j, int delay) {
    Soldier *sol = &list[k];
    float angle = atan2(i - sol->i, j - sol->j);
    sol->dx = cos(angle) * speed;
    sol->dy = sin(angle) * speed;
    sol->i = i;
    sol->j = j;
    sol->delay = delay;
}

void manage_conflict(int k) {
    Soldier *sol = &list[k];
    for (int k2 = listsize - 1; k2 >= 0; k2--) {
        Soldier *sol2 = &list[k2];
        if ((sol->owner != sol2->owner) && sol2->owner &&
            (is_moving(k2) || (sol->i == sol2->i && sol->j == sol2->j)) &&
            fabs(sol->x - sol2->x) < eps && fabs(sol->y - sol2->y) < eps) {
            remove_soldier(k);
            if (!player_has_potion(sol2->owner, TYPE_NO_ATTACK) ||
                is_moving(k2)) {
                remove_soldier(k2);
            }
            break;
        }
    }
}

void move_soldier(int k) {
    Soldier *sol = &list[k];
    if (others_has_potion(sol->owner, TYPE_FREEZE)) {
        return;
    }
    if (sol->delay > 0) {
        sol->delay--;
    }
    manage_conflict(k);
    manage_potion_cross(k);
    if (is_inside(k, sol->i, sol->j, eps)) {
        sol->dx = 0;
        sol->dy = 0;
        set_locatoin(k, sol->i, sol->j);
    }

    float factor = 1;
    if (player_has_potion(sol->owner, TYPE_HIGH_SPEED)) {
        factor = 2;
    }
    if (others_has_potion(sol->owner, TYPE_LOW_SPEED)) {
        factor = 0.5;
    }

    if (sol->delay == 0) {
        sol->x += factor * sol->dx;
        sol->y += factor * sol->dy;
    }
}
