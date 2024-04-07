#include "pacman.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>

void initCMan(C_man_FSM *fsm) {
    fsm->currentState = NORMAL_MODE;
    fsm->immuneSteps = 5;
}

scoreboard* create_scoreboard(void) {
    scoreboard* sb = malloc(sizeof(scoreboard)); 
    if (sb != NULL) {
        sb->score = 0;
        sb->level = 1;
        sb->immunity = malloc(sizeof(C_man_FSM)); 
        if (sb->immunity != NULL) {
            initCMan(sb->immunity); 
        }
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

entity** create_ghosts() {
    entity **ghosts = malloc(NUM_GHOSTS * sizeof(entity*));
    if (ghosts != NULL) {
        for (int i = 0; i < NUM_GHOSTS; i++) {
            ghosts[i] = create_entity(8, 8);
        }
    }
    return ghosts;
}

void set_point_map(int map[GRIDSIZE][GRIDSIZE],scoreboard *scoreboard) {
    int i, j;
    int num_big_points = 8;
    int count = 0;
 
    
     for (i = 0; i < GRIDSIZE; i++) {
        for (j = 0; j < GRIDSIZE; j++) {
            map[i][j] = 1;
        }
     }
    for (i = 1; i < GRIDSIZE-1; i++) {
        for (j = 1; j < GRIDSIZE-1; j++) {
            if(i==1 && j==1){
                 map[i][j] = 2;
               continue;
            }
            if(i==1){
                map[i][j] = 2;
                continue;
            }
            int random_num = rand() % 20;
           
            if (random_num > 3) {
                map[i][j] = 2; // movable path
                count += 1;
            } else {
                map[i][j] = 1; // obstacle
                
            }
        }
    }
  

    for (i = 1; i < GRIDSIZE-1; i++) {
        for (j = 1; j < GRIDSIZE-1; j++) {
            if (map[i-1][j] == 1 && map[i+1][j] == 1 && map[i][j-1] == 1 && map[i][j+1] == 1) {
                map[i][j] = 1; // If all neighboring cells contain obstacles, place an obstacle
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

void print_map(int map[GRIDSIZE][GRIDSIZE], entity *player, entity **ghosts) {
    int i, j, k;
    for (i = 0; i < GRIDSIZE; i++) {
        for (j = 0; j < GRIDSIZE; j++) {
            if ((i == ghosts[0]->pos_x && j == ghosts[0]->pos_y && ghosts[0]->is_active) ||
                (i == ghosts[1]->pos_x && j == ghosts[1]->pos_y && ghosts[1]->is_active) ||
                (i == ghosts[2]->pos_x && j == ghosts[2]->pos_y && ghosts[2]->is_active) ||
                (i == ghosts[3]->pos_x && j == ghosts[3]->pos_y && ghosts[3]->is_active)) {
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
    printf("IMMUNITY: %d\n", scoreboard->immunity->immuneSteps);
}

void move_entity(entity *ent, entity **ghosts, int map[GRIDSIZE][GRIDSIZE], char direction) {
    int next_x = ent->pos_x;
    int next_y = ent->pos_y;

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
        int collision = 0;
        for (int i = 0; i < NUM_GHOSTS; i++) {
            if (ghosts[i] != ent && ghosts[i]->pos_x == next_x && ghosts[i]->pos_y == next_y) {
                collision = 1; 
                break;
            }
        }
        if (!collision) {
            ent->pos_x = next_x;
            ent->pos_y = next_y;
        }
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
            if (map[next_x][next_y] == 2) {
                map[next_x][next_y] = 0; 
                scoreboard->score++;
                scoreboard->immunity->immuneSteps--;  
            } else if (map[next_x][next_y] == 3) {
                map[next_x][next_y] = 0; 
                scoreboard->score++;
                scoreboard->score++;
                scoreboard->immunity->immuneSteps += 5;
            } else if (map[next_x][next_y] == 4) {
                if (next_x == 2 && next_y == 2) {
                    next_x = 7; next_y = 7; 
                } else if (next_x == 7 && next_y == 7) {
                    next_x = 2; next_y = 2; 
                }
                scoreboard->immunity->immuneSteps--;  
            } else {
                scoreboard->immunity->immuneSteps--;  
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

void handleCManState(GameStateFsm *fsm) {
    if (fsm->scoreboard->immunity->currentState == NORMAL_MODE) {
        if (fsm->scoreboard->immunity->immuneSteps < 0) {
            fsm->scoreboard->immunity->immuneSteps = 0;
        }
        else if (fsm->scoreboard->immunity->immuneSteps > 0) {
            fsm->scoreboard->immunity->currentState = POWER_MODE;
        }
    }
    else if (fsm->scoreboard->immunity->currentState == POWER_MODE && fsm->scoreboard->immunity->immuneSteps == 0) {  
        fsm->scoreboard->immunity->currentState = NORMAL_MODE;  
    } 
}

void initGameStateFsm(GameStateFsm *fsm) {
    fsm->currentGameState = Menu;
    fsm->scoreboard = create_scoreboard();
    set_point_map(fsm->map,fsm->scoreboard);
    fsm->player = create_entity(1, 1);
    fsm->ghosts = create_ghosts();
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
    int i;
    

    if (input == 'q') {
        fsm->currentGameState = End;
    } else if (input == ' ') {
        fsm->currentGameState = Paused;
        printf("Game Paused. Press Spacebar to resume...\n");
    } else if (input == 'w' || input == 'a' || input == 's' || input == 'd') {
        move_player(fsm->player, fsm->map, input, fsm->scoreboard);
    }

    static int count = 0;
    int ghost_move_interval = 3; 
    int player_x, player_y, ghost_x, ghost_y, distance_x, distance_y;
    if (count++ >= ghost_move_interval) {
        for (i = 0; i < NUM_GHOSTS; i++) {
            if (fsm->ghosts[i]->is_active) {
                player_x = fsm->player->pos_x;
                player_y = fsm->player->pos_y;
                ghost_x = fsm->ghosts[i]->pos_x;
                ghost_y = fsm->ghosts[i]->pos_y;

                distance_x = abs(player_x - ghost_x);
                distance_y = abs(player_y - ghost_y);

                if (distance_x <= 4 && distance_y <= 4) {
                    if (distance_x > distance_y) {
                        if (player_x < ghost_x && fsm->map[ghost_x - 1][ghost_y] != 1) {
                            move_entity(fsm->ghosts[i], fsm->ghosts, fsm->map, 'w'); 
                        } else if (player_x > ghost_x && fsm->map[ghost_x + 1][ghost_y] != 1) {
                            move_entity(fsm->ghosts[i], fsm->ghosts, fsm->map, 's'); 
                        }
                    }
                    if (ghost_x == fsm->ghosts[i]->pos_x) { 
                        if (player_y < ghost_y && fsm->map[ghost_x][ghost_y - 1] != 1) {
                            move_entity(fsm->ghosts[i], fsm->ghosts, fsm->map, 'a'); 
                        } else if (player_y > ghost_y && fsm->map[ghost_x][ghost_y + 1] != 1) {
                            move_entity(fsm->ghosts[i], fsm->ghosts, fsm->map, 'd'); 
                        }
                    }
                } else {
                    move_entity(fsm->ghosts[i], fsm->ghosts, fsm->map, get_random_direction());
                }
            }
        }
        count = 0; 
    }

    handleCManState(fsm);
    for (i = 0; i < NUM_GHOSTS; i++) {
        if (check_collision(fsm->player, fsm->ghosts[i])) {
            if (fsm->scoreboard->immunity->currentState == NORMAL_MODE && fsm->ghosts[i]->is_active) {
                printf("\nGame Over! You've been caught by the ghost!\n");
                printf("Do you want to start over or exit?\nPress 'y' to start over and press 'x' to exit\n");
                fsm->currentGameState = GameOver;
                break;
            } else {
                fsm->ghosts[i]->is_active = 0;
                fsm->scoreboard->score += 10;
            }
        }
    }
        
/*    
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
*/

    if (all_points_eaten(fsm->map)) {
        // fsm->currentGameState = End;
            
         // printf("\nGame Over!\nCongratulations, you've eaten all the points!\n");
        set_point_map(fsm->map, fsm->scoreboard);
        fsm->player->pos_x = 1;
        fsm->player->pos_y = 1;
        fsm->scoreboard->level++;

        for (i = 0; i < NUM_GHOSTS; i++) {
            fsm->ghosts[i]->pos_x = 8;
            fsm->ghosts[i]->pos_y = 8;
            fsm->ghosts[i]->is_active = 1;
        }
/*
        fsm->ghost->pos_x = 8;
        fsm->ghost->pos_y = 8;
        fsm->ghost->is_active = 1;
*/
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

void free_ghosts(entity **ghosts) {
    for (int i = 0; i < NUM_GHOSTS; i++) {
        free(ghosts[i]);
    }
    free(ghosts);
}

int main() {
    static int counter = 0;
    // int immunity_counter = 5;
    int ghost_move_interval = 3;

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
    int i;
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
                print_map(fsm.map, fsm.player, fsm.ghosts);
            } else if (fsm.currentGameState == GameOver){
                handleGameOverState(&fsm,input);
            }

        if (counter++ >= ghost_move_interval) {
            for (i = 0; i < NUM_GHOSTS; i++) {
                move_entity(fsm.ghosts[i], fsm.ghosts, fsm.map, get_random_direction());    
            }
            counter = 0;
        }


        // if (immunity++ >= immunity_counter) {
        //     immunity = 0;
        //     if (fsm.scoreboard->immunity > 0) {
        //         fsm.scoreboard->immunity--;
        //     }
        // }


       sleep_microseconds(100000);
    }

    set_canonical_mode();
    fprintf(file, "%s, %d\n", username, fsm.scoreboard->score);
    fclose(file);

    printf("Data written successfully to data.csv\n");
    free(fsm.player);
    free(fsm.scoreboard->immunity);
    free(fsm.scoreboard);
    free_ghosts(fsm.ghosts);

    return 0;
}


   



