typedef struct {
    int owner;
    float x;
    float y;
    float dx;
    float dy;
    int i;
    int j;
    int delay;
} Soldier;

const int steps = 100;
int cell_size = 120;
int speed = 3;
int eps = 3;

// ---------------------------- Soldier List -----------------------------

Soldier list[1500];
int listsize = 0;

void init_soldier(int k, int owner, int i, int j) {
    Soldier *sol = &list[k];
    sol->owner = owner;
    sol->i = i;
    sol->j = j;
    sol->x = (j + 0.5) * cell_size;
    sol->y = (i + 0.5) * cell_size;
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

void set_soldier_target(int k, int i, int j, int delay) {
    Soldier *sol = &list[k];
    float angle = atan2(i - sol->i, j - sol->j);
    sol->dx = cos(angle) * speed;
    sol->dy = sin(angle) * speed;
    sol->i = i;
    sol->j = j;
    sol->delay = delay;
}

void move_soldier(int k) {
    Soldier *sol = &list[k];
    if (sol->delay > 0) {
        sol->delay--;
    }
    // check conflicts and kill
    for (int k2 = 0; k2 < listsize; k2++) {
        Soldier *sol2 = &list[k2];
        if ((sol->owner != sol2->owner) && sol2->owner &&
            fabs(sol->x - sol2->x) < eps && fabs(sol->y - sol2->y) < eps) {
            remove_soldier(k);
            remove_soldier(k2);
            break;
        }
    }
    if (fabs(sol->x - (sol->j + 0.5) * cell_size) < eps &&
        fabs(sol->y - (sol->i + 0.5) * cell_size) < eps) {
        sol->dx = 0;
        sol->dy = 0;
    }
    if (sol->delay == 0) {
        sol->x += sol->dx;
        sol->y += sol->dy;
    }
}
