#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#define GRIDSIZE 15
#define MAX_LINE_LENGTH 1024
#define NUM_LINES_TO_READ 3
#define MAX_ENTRIES 1000
#define MAX_NAME_LENGTH 50


typedef struct {
    int max_score;
    int score;
    int level;
    int immunity;
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
    entity *ghost;
    scoreboard *scoreboard;
    int map[GRIDSIZE][GRIDSIZE];
} GameStateFsm;

scoreboard* create_scoreboard(void) {
    scoreboard* sb = malloc(sizeof(scoreboard)); 
    if (sb != NULL) {
        sb->score = 0;
        sb->level = 1;
        sb->immunity = 0;
    }
    return sb;
}

entity* create_entity(int x, int y) {
    entity *e = malloc(sizeof(entity));
    if (e != NULL) {
        e->pos_x = x;
        e->pos_y = y;
        e->is_active = 1;
    }
    return e;
}

void set_point_map(int map[GRIDSIZE][GRIDSIZE],scoreboard *scoreboard) {
    int i, j;
    int num_big_points = 10;
    int count = 0;
    for (i = 0; i < GRIDSIZE; i++) {
        map[i][0] = 1; 
        map[i][GRIDSIZE-1] = 1; 
        map[0][i] = 1; 
        map[GRIDSIZE-1][i] = 1; 
    }
    

    for (i = 1; i < GRIDSIZE-1; i++) {
        for (j = 1; j < GRIDSIZE-1; j++) {
            if(i==1 && j==1){
               continue;
               
            }
            int random_num = rand() % 10;
            // Assign 1 to approximately 20% of the cells
            if (random_num < 2) {
                map[i][j] = 1; // obstacle
            } else {
                map[i][j] = 2; // movable path
                count += 1;
            }
        }
    }

    srand(time(NULL)); 
    while (num_big_points > 0) {
        int x = rand() % 8 + 1; 
        int y = rand() % 8 + 1;
        if (map[x][y] == 2) {  
            map[x][y] = 3;
            num_big_points--;
        }
    }
    scoreboard->max_score = count;
}

void set_teleportation_points(int map[GRIDSIZE][GRIDSIZE]) {
    map[2][2] = 4; 
    map[7][7] = 4; 
}

void print_map(int map[GRIDSIZE][GRIDSIZE], entity *player, entity *ghost) {
    int i, j;
    for (i = 0; i < GRIDSIZE; i++) {
        for (j = 0; j < GRIDSIZE; j++) {
            if (i == ghost->pos_x && j == ghost->pos_y && ghost->is_active) {
                printf("G ");
            } else if (i == player->pos_x && j == player->pos_y) {
                printf("C ");
            } else if (map[i][j] == 1) {
                printf("# "); 
            } else if (map[i][j] == 2) {
                printf(". "); 
            } else if (map[i][j] == 3) {
                printf("o "); 
            } else if (map[i][j] == 4) {
                printf("T "); 
            }else {
                printf("  "); 
            }
        }
        printf("\n");
    }
}

void print_score_board(scoreboard *scoreboard) {
    printf("LEVEL: %d\n", scoreboard->level);
    printf("SCORE: %d\n", scoreboard->score);
    printf("MAX SCORE: %d\n", scoreboard->max_score);
    printf("IMMUNITY: %d\n", scoreboard->immunity);
}

void move_entity(entity *ent, int map[GRIDSIZE][GRIDSIZE], char direction) {
    int next_x, next_y;
    next_x = ent->pos_x;
    next_y = ent->pos_y;
    switch (direction) {
        case 'w': 
            next_x--;
            break;
        case 's': 
            next_x++;
            break;
        case 'a': 
            next_y--;
            break;
        case 'd':
            next_y++;
            break;
    }

    if (next_x >= 0 && next_x < GRIDSIZE && next_y >= 0 && next_y < GRIDSIZE && map[next_x][next_y] != 1) {
        ent->pos_x = next_x;
        ent->pos_y = next_y;
    }
}

void move_player(entity *player, int map[GRIDSIZE][GRIDSIZE], char direction, scoreboard *scoreboard) {
    int next_x,next_y;

    next_x = player->pos_x, next_y = player->pos_y;
    switch (direction) {
        case 'w': 
            next_x--;
            break;
        case 's': 
            next_x++;
            break;
        case 'a': 
            next_y--;
            break;
        case 'd':
            next_y++;
            break;
    }

    
    if (next_x >= 0 && next_x < GRIDSIZE && next_y >= 0 && next_y < GRIDSIZE && map[next_x][next_y] != 1) {
        if (map[next_x][next_y] != 1) { 
            if (map[next_x][next_y] == 2) {
                map[next_x][next_y] = 0; 
                scoreboard->score++;
            }
            else if (map[next_x][next_y] == 3) {
                map[next_x][next_y] = 0; 
                scoreboard->score++;
                scoreboard->score++;
                scoreboard->immunity+=5;
            }
            else if (map[next_x][next_y] == 4) {
            if (next_x == 2 && next_y == 2) {
                next_x = 7; next_y = 7; 
            } else if (next_x == 7 && next_y == 7) {
                next_x = 2; next_y = 2; 
            }
        }
        }
        
        player->pos_x = next_x;
        player->pos_y = next_y;
    }

}

char get_random_direction() {
    int randDir = rand() % 4;
    switch (randDir) {
        case 0: return 'w';
        case 1: return 's';
        case 2: return 'a';
        case 3: return 'd';
    }
    return 'w';
}

int check_collision(entity *player, entity *ghost) {
    if (player->pos_x == ghost->pos_x && player->pos_y == ghost->pos_y) {
        return 1;
    }
    return 0;
}

int all_points_eaten(int map[GRIDSIZE][GRIDSIZE]) {
    int i,j;
    for (i = 0; i < GRIDSIZE; i++) {
        for (j = 0; j < GRIDSIZE; j++) {
            if (map[i][j] == 2 || map[i][j] == 3) { 
                return 0; 
            }
        }
    }
    return 1; 
}

void initGameStateFsm(GameStateFsm *fsm) {
    fsm->currentGameState = Menu;
    fsm->scoreboard = create_scoreboard();
    set_point_map(fsm->map,fsm->scoreboard);
    fsm->player = create_entity(1, 1);
    fsm->ghost = create_entity(8, 8);
    set_teleportation_points(fsm->map);
}

void handleMenuState(GameStateFsm *fsm, char input) {
    if (input == '\n') {
        fsm->currentGameState = Playing;
        system("clear");
    }else if (input == 'q'){
        fsm->currentGameState = End;
    }
    /*
        1. can add case to quit from menu
        2. can add option to reset data file, clear max score, etc.    
    */
}
void handlePlayingState(GameStateFsm *fsm, char input) {
    if (input == 'q') {
        fsm->currentGameState = End;
    } else if (input == ' ') {
        fsm->currentGameState = Paused;
        printf("Game Paused. Press Spacebar to resume...\n");
    } else if (input == 'w' || input == 'a' || input == 's' || input == 'd') {
        move_player(fsm->player, fsm->map, input, fsm->scoreboard);
    }

    if (check_collision(fsm->player, fsm->ghost)) {
        if (fsm->scoreboard->immunity == 0 && fsm->ghost->is_active) {
            printf("\nGame Over! You've been caught by the ghost!\n");
            printf("Do you want to start over or exit?\nPress 'y' to start over and press 'x' to exit\n");
            fsm->currentGameState = GameOver;

        } else {
            fsm->ghost->is_active = 0;
            fsm->scoreboard->score += 10;
        }
    }

    if (all_points_eaten(fsm->map)) {
        // fsm->currentGameState = End;
            
         // printf("\nGame Over!\nCongratulations, you've eaten all the points!\n");
        set_point_map(fsm->map, fsm->scoreboard);
        fsm->player->pos_x = 1;
        fsm->player->pos_y = 1;
       
        fsm->ghost->pos_x = 8;
        fsm->ghost->pos_y = 8;
        fsm->scoreboard->level++;
        fsm->ghost->is_active = 1;
    }
}


/*
    if (count++ >= ghost_move_interval) {
        move_entity(fsm->ghost, fsm->map, get_random_direction());
        count = 0;
        if (fsm->scoreboard->immunity > 0) {
            fsm->scoreboard->immunity--;
        }
    }
*/

void handlePausedState(GameStateFsm *fsm, char input) {
    if (input == ' ') {
        fsm->currentGameState = Playing;
        system("clear");
    }
}

void handleGameOverState(GameStateFsm *fsm, char input) {
    if (input == 'y' || input == 'Y') {
        initGameStateFsm(fsm); 
        fsm->currentGameState = Playing; 
    } else if(input == 'x' || input =='X'){
        fsm->currentGameState = End;
    }
}

void processGameState(GameStateFsm *fsm, char input) {
        switch (fsm->currentGameState) {
            case Menu:
                handleMenuState(fsm, input);
                break;
            case Playing:
                handlePlayingState(fsm, input);
                break;
            case Paused:
                handlePausedState(fsm, input);
                break;
            case GameOver:
                handleGameOverState(fsm,input);
            case End:
                break;
        }
    }


void sleep_microseconds(long microseconds) {
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 50000000; // 50 milliseconds
    nanosleep(&ts, NULL);
}

void set_noncanonical_mode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    term.c_cc[VMIN] = 0; 
    term.c_cc[VTIME] = 1; 
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void set_canonical_mode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= (ICANON | ECHO); 
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

struct Entry {
    char username[MAX_NAME_LENGTH];
    int score;
};

void swap(struct Entry *a, struct Entry *b) {
    struct Entry temp = *a;
    *a = *b;
    *b = temp;
}

void bubbleSort(struct Entry arr[], int n) {
    int i, j;
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - i - 1; j++) {
            if (arr[j].score < arr[j + 1].score) {
                swap(&arr[j], &arr[j + 1]);
            }
        }
    }
}

int main() {
    static int counter = 0;
    int ghost_move_interval = 5;
    char username[50];
    printf("Enter a username:\n");
    scanf("%s", username);
   

    FILE *file = fopen("data.csv", "r");
    if (file == NULL) {
        printf("Could not open the file.\n");
        return 1;
    }

    struct Entry entries[MAX_ENTRIES];
    int count = 0;

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        char *token = strtok(line, ",");
        strcpy(entries[count].username, token);
        token = strtok(NULL, ",");
        entries[count].score = atoi(token);
        count++;
    }

    fclose(file);

    bubbleSort(entries, count);

    file = fopen("data.csv", "a+");

    GameStateFsm fsm;
    char input;
    initGameStateFsm(&fsm);
    
    
    set_noncanonical_mode();
    system("clear");
    printf("Welcome to C-man %s\n",username);
    printf("Play with ONLY W, A, S, D\n");
    printf("Press Enter to start the game...\n");
    printf("Press Spacebar to pause and resume the game...\n");
    printf("Press q to quit the game...\n");
    printf("Scoreboard: \n");
    for (int i = 0; i < NUM_LINES_TO_READ; i++) {
        if(strcmp(entries[i].username,"")){
         printf("%s: %d\n", entries[i].username, entries[i].score);
        }
       
    }

    while (fsm.currentGameState != End) {
        if (read(STDIN_FILENO, &input, 1) > 0) {
            processGameState(&fsm, input);
        }

        if (fsm.currentGameState == Playing) {
            system("clear");
            print_score_board(fsm.scoreboard);
            print_map(fsm.map, fsm.player, fsm.ghost);
        } else if (fsm.currentGameState == GameOver){
            handleGameOverState(&fsm,input);
        }
        if (counter++ >= ghost_move_interval) {
            move_entity(fsm.ghost, fsm.map, get_random_direction());
            counter = 0;
            if (fsm.scoreboard->immunity > 0) {
                fsm.scoreboard->immunity--;
            }
        }
       sleep_microseconds(100000);
    }

    set_canonical_mode();
    fprintf(file, "%s, %d\n", username, fsm.scoreboard->score);
    fclose(file);

    printf("Data written successfully to data.csv\n");
    free(fsm.player);
    free(fsm.scoreboard);

    return 0;
}


   



