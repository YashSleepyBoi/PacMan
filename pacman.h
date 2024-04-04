#define GRIDSIZE 15
#define MAX_LINE_LENGTH 1024
#define NUM_LINES_TO_READ 3
#define MAX_ENTRIES 1000
#define MAX_NAME_LENGTH 50
#define NUM_GHOSTS 4

typedef enum {
    NORMAL_MODE, 
    POWER_MODE 
} C_man_State;

typedef struct {
    C_man_State currentState;
    int immuneSteps;
} C_man_FSM;

typedef struct {
    int max_score;
    int score;
    int level;
    C_man_FSM *immunity;
} scoreboard;

typedef struct{
    int pos_x;
    int pos_y;
    int is_active;
} entity;

typedef enum {
    Menu,
    Playing,
    Paused,
    GameOver,
    End
} GameState;

typedef struct {
    GameState currentGameState;
    entity *player;
    entity **ghosts;
    scoreboard *scoreboard;
    int map[GRIDSIZE][GRIDSIZE];
} GameStateFsm;

