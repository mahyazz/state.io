// ---------------------------- Soldiers -----------------------------

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

extern Soldier list[];
extern int listsize;

// ---------------------------- Functions -----------------------------

void add_soldier(int owner, int i, int j);
int is_inside(int k, int i, int j, float r);
void move_soldier(int k);
int is_moving(int k);
void set_soldier_target(int k, int i, int j, int delay);
