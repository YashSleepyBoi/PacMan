#define _POSIX_C_SOURCE 199309L

#include "pacman.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <regex.h>

void initCMan(C_man_FSM *fsm) {
    fsm->currentState = NORMAL_MODE;
    fsm->immuneSteps = 0;
}

scoreboard* create_scoreboard(void) {
    /* Allocate memory for scoreboard */
    scoreboard* sb = malloc(sizeof(scoreboard)); 

    if (sb != NULL) {
        /* Initialize score to 0 and level to 1 */
        sb->score = 0;
        sb->level = 1;
        /* Allocate memory for immunity FSM */
        sb->immunity = malloc(sizeof(C_man_FSM)); 
        if (sb->immunity != NULL) {
            /* Initialize C_man FSM */
            initCMan(sb->immunity); 
        }
    }
    return sb;
}

entity* create_entity(int x, int y) {
    /* Allocate memory for entity struct */
    entity *e = malloc(sizeof(entity));

    if (e != NULL) {
        /* Initialize x and y positions and set entity as active */
        e->pos_x = x;
        e->pos_y = y;
        e->is_active = 1;
    }
    return e;
}

entity** create_ghosts(void) {
    int i;
    /* Allocate memory for array of ghost entities */
    entity **ghosts = malloc(NUM_GHOSTS * sizeof(entity*));
    
    if (ghosts != NULL) {
        for (i = 0; i < NUM_GHOSTS; i++) {
            /* Create ghost entities at initial position */
            ghosts[i] = create_entity(8, 8);
        }
    }
    return ghosts;
}

void set_point_map(int map[GRIDSIZE][GRIDSIZE], scoreboard *scoreboard) {
    int i, j;
    int num_big_points = 8;
    int count = 0;
    int random_num;
    
    /* Set all cells in map to 1 as barriers */
    for (i = 0; i < GRIDSIZE; i++) {
        for (j = 0; j < GRIDSIZE; j++) {
            map[i][j] = 1;
        }
    }   

    /* Create paths (with tiny cookies) and obstacles in the map */
    for (i = 1; i < GRIDSIZE-1; i++) {
        for (j = 1; j < GRIDSIZE-1; j++) {
            if(i==1){
                map[i][j] = 2;
                continue;
            }
            random_num = rand() % 20;
           
            if (random_num > 3) {
                /* movable path */
                map[i][j] = 2; 
                count += 1;
            } else {
                /* obstacle */
                map[i][j] = 1; 
                
            }
        }
    }
  
    /* Handle isolated obstacles */
    for (i = 1; i < GRIDSIZE-1; i++) {
        for (j = 1; j < GRIDSIZE-1; j++) {
            if (map[i-1][j] == 1 && map[i+1][j] == 1 && map[i][j-1] == 1 && map[i][j+1] == 1) {
                /* If all neighboring cells contain obstacles, place an obstacle */
                map[i][j] = 1;
            } 
        }
    }
    for (i = GRIDSIZE-1; i >0; i--) {
        for (j = GRIDSIZE; j >0; j--) {
            if (map[i-1][j] == 1 && map[i+1][j] == 1 && map[i][j-1] == 1 && map[i][j+1] == 1) {
                /* If all neighboring cells contain obstacles, place an obstacle */
                map[i][j] = 1;
            } 
        }
    }
    for (i = 1; i < GRIDSIZE-1; i++) {
        for (j = 1; j < GRIDSIZE-1; j++) {
            if (map[i-1][j] == 1 && map[i+1][j] == 1 && map[i][j-1] == 1 && map[i][j+1] == 1) {
                /* If all neighboring cells contain obstacles, place an obstacle */
                map[i][j] = 1;
            } 
        }
    }

    /* Generate big cookies randomly */
    srand(time(NULL)); 
    while (num_big_points > 0) {
        int x = rand() % 13 + 1; 
        int y = rand() % 13 + 1;
        if (map[x][y] == 2) {  
            map[x][y] = 3;
            num_big_points--;
        }
    }
    map[1][1] = 0;
    scoreboard->max_score = count;
}

void set_teleportation_points(int map[GRIDSIZE][GRIDSIZE]) {
    /* Set teleportation points in map */
    map[2][2] = 4; 
    map[12][12] = 4; 
}

void print_map(int map[GRIDSIZE][GRIDSIZE], entity *player, entity **ghosts, scoreboard *scoreboard) {
    int i, j;

    for (i = 0; i < GRIDSIZE; i++) {
        for (j = 0; j < GRIDSIZE; j++) {
            /* Ghost */
            if ((i == ghosts[0]->pos_x && j == ghosts[0]->pos_y && ghosts[0]->is_active) ||
                (i == ghosts[1]->pos_x && j == ghosts[1]->pos_y && ghosts[1]->is_active) ||
                (i == ghosts[2]->pos_x && j == ghosts[2]->pos_y && ghosts[2]->is_active) ||
                (i == ghosts[3]->pos_x && j == ghosts[3]->pos_y && ghosts[3]->is_active)) {
                printf("G ");
            } /* Powered-up C-Man */
            else if (i == player->pos_x && j == player->pos_y && scoreboard->immunity->currentState == POWER_MODE) {
                printf("D ");
            } /* Normal C-Man */ 
            else if (i == player->pos_x && j == player->pos_y && scoreboard->immunity->currentState == NORMAL_MODE) {
                printf("C ");
            } /* Barrier */
            else if (map[i][j] == 1) {
                printf("# "); 
            } /* Tiny cookie */
            else if (map[i][j] == 2) {
                printf(". "); 
            } /* Big cookie */
            else if (map[i][j] == 3) {
                printf("o "); 
            } /* Teleportation point */ 
            else if (map[i][j] == 4) {
                printf("T "); 
            }else {
                printf("  "); 
            }
        }
        printf("\n");
    }
}

void print_score_board(scoreboard *scoreb) {
    printf("LEVEL: %d\n", scoreb->level);
    printf("SCORE: %d\n", scoreb->score);
    printf("MAX SCORE: %d\n", scoreb->max_score);
    printf("IMMUNITY: %d\n", scoreb->immunity->immuneSteps);
}

int check_collision(entity *ent1, entity *ent2) {
    /* Check for collision between two entities */
    if (ent1->pos_x == ent2->pos_x && ent1->pos_y == ent2->pos_y) {
        return 1;
    }
    return 0;
}

void move_ghost(entity *ghost, entity **ghosts, int map[GRIDSIZE][GRIDSIZE], char direction) {
    int i;
    int collision = 0;
    int next_x = ghost->pos_x;
    int next_y = ghost->pos_y;

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

    /* Check if next ghost move is within boundaries and not an obstacle */
    if (next_x >= 0 && next_x < GRIDSIZE && next_y >= 0 && next_y < GRIDSIZE && map[next_x][next_y] != 1) {
        for (i = 0; i < NUM_GHOSTS; i++) {
            /* Check if ghost collides with any other ghosts */
            if (ghosts[i] != ghost && ghosts[i]->pos_x == next_x && ghosts[i]->pos_y == next_y) {
                collision = 1; 
                break;
            }
        }
        if (!collision) {
            ghost->pos_x = next_x;
            ghost->pos_y = next_y;
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

    /* Check if next move is within boundaries and not an obstacle */
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
            } /* Handle teleportation points */ 
            else if (map[next_x][next_y] == 4) {
                if (next_x == 2 && next_y == 2) {
                    next_x = 12; next_y = 12; 
                } else if (next_x == 12 && next_y == 12) {
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

char get_random_direction(void) {
    int randDir = rand() % 4;

    switch (randDir) {
        case 0: return 'w';
        case 1: return 's';
        case 2: return 'a';
        case 3: return 'd';
    }
    return 'w';
}

int all_points_eaten(int map[GRIDSIZE][GRIDSIZE]) {
    int i, j;

    /* Check if all points eaten in map */
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
        /* Immunity steps cannot be negative */
        if (fsm->scoreboard->immunity->immuneSteps < 0) {
            fsm->scoreboard->immunity->immuneSteps = 0;
        } /* Switch to POWER_MODE */
        else if (fsm->scoreboard->immunity->immuneSteps > 0) {
            fsm->scoreboard->immunity->currentState = POWER_MODE;
        }
    } /* Switch to NORMAL MODE */
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
}

void handlePlayingState(GameStateFsm *fsm, char input) {
    int i;
    static int count = 0;
    int ghost_move_interval = 3; 
    int player_x, player_y, ghost_x, ghost_y, distance_x, distance_y;

    /* Handle input keys from player */
    if (input == 'q') {
        fsm->currentGameState = End;
    } else if (input == ' ') {
        fsm->currentGameState = Paused;
        printf("Game Paused. Press Spacebar to resume...\n");
    } else if (input == 'w' || input == 'a' || input == 's' || input == 'd') {
        move_player(fsm->player, fsm->map, input, fsm->scoreboard);
    }
    
    if (count++ >= ghost_move_interval) {
        for (i = 0; i < NUM_GHOSTS; i++) {
            if (fsm->ghosts[i]->is_active) {
                player_x = fsm->player->pos_x;
                player_y = fsm->player->pos_y;
                ghost_x = fsm->ghosts[i]->pos_x;
                ghost_y = fsm->ghosts[i]->pos_y;

                distance_x = abs(player_x - ghost_x);
                distance_y = abs(player_y - ghost_y);

                /* Ghost move towards player when player is in closer radius to ghost */
                if (distance_x <= 6 && distance_y <= 6) {
                    if (distance_x > distance_y) {
                        if (player_x < ghost_x && fsm->map[ghost_x - 1][ghost_y] != 1) {
                            move_ghost(fsm->ghosts[i], fsm->ghosts, fsm->map, 'w'); 
                        } else if (player_x > ghost_x && fsm->map[ghost_x + 1][ghost_y] != 1) {
                            move_ghost(fsm->ghosts[i], fsm->ghosts, fsm->map, 's'); 
                        }
                    }
                    if (ghost_x == fsm->ghosts[i]->pos_x) { 
                        if (player_y < ghost_y && fsm->map[ghost_x][ghost_y - 1] != 1) {
                            move_ghost(fsm->ghosts[i], fsm->ghosts, fsm->map, 'a'); 
                        } else if (player_y > ghost_y && fsm->map[ghost_x][ghost_y + 1] != 1) {
                            move_ghost(fsm->ghosts[i], fsm->ghosts, fsm->map, 'd'); 
                        }
                    }
                } else {
                    move_ghost(fsm->ghosts[i], fsm->ghosts, fsm->map, get_random_direction());
                }
            }
        }
        count = 0; 
    }

    handleCManState(fsm);
    for (i = 0; i < NUM_GHOSTS; i++) {
        /* Check for collision between C-man and ghost, and check if C-man is in POWER_MODE */
        if (check_collision(fsm->player, fsm->ghosts[i])) {
            
            if (fsm->scoreboard->immunity->currentState == NORMAL_MODE && fsm->ghosts[i]->is_active) {
                /*printf("\nGame Over! You've been caught by the ghost!\n");
                printf("Do you want to start over or exit?\nPress 'y' to start over and press 'x' to exit\n");*/
                fsm->currentGameState = GameOver;
                break;
            } else {
                fsm->ghosts[i]->is_active = 0;
                fsm->scoreboard->score += 10;
            }
        }
    }

    /* Proceed to next stage */
    if (all_points_eaten(fsm->map)) {
        set_point_map(fsm->map, fsm->scoreboard);
        set_teleportation_points(fsm->map);
        fsm->player->pos_x = 1;
        fsm->player->pos_y = 1;
        fsm->scoreboard->level++;

        /* Generate new ghosts */
        for (i = 0; i < NUM_GHOSTS; i++) {
            fsm->ghosts[i]->pos_x = 8;
            fsm->ghosts[i]->pos_y = 8;
            fsm->ghosts[i]->is_active = 1;
        }
    }
}

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
    ts.tv_nsec = 50000000; 
    nanosleep(&ts, NULL);
}

void set_noncanonical_mode(void) {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    term.c_cc[VMIN] = 0; 
    term.c_cc[VTIME] = 1; 
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void set_canonical_mode(void) {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= (ICANON | ECHO); 
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

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
    int i;
    for (i = 0; i < NUM_GHOSTS; i++) {
        free(ghosts[i]);
    }
    free(ghosts);
}
typedef struct {
    char username[50];
    int score;
} Entry;

int main(void) {
    int i;
    int count = 0;
    static int counter = 0; /* Counter for ghost movement interval */
    int ghost_move_interval = 3;
    int gameoverprinted;
    char input;
    char line[1024];
    char username[50];
  
    FILE *file; /* file pointer for CSV file */
    GameStateFsm fsm;
    struct Entry entries[MAX_ENTRIES];
  

    regex_t regex;
    regcomp(&regex, "([^,]+), ([0-9]+)", REG_EXTENDED);

    printf("Enter a username:\n");
    scanf("%s", username);


    file = fopen("data.csv", "r");
    if (file == NULL) {
        printf("Could not open the file.\n");
        return 1;
    }

    /* Parser*/
   while (fgets(line, sizeof(line), file)) {
        regmatch_t matches[3];
        if (regexec(&regex, line, 3, matches, 0) == 0) {
            char score_str[10];
            strncpy(entries[count].username, line + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
            entries[count].username[matches[1].rm_eo - matches[1].rm_so] = '\0';
            strncpy(score_str, line + matches[2].rm_so, matches[2].rm_eo - matches[2].rm_so);
            score_str[matches[2].rm_eo - matches[2].rm_so] = '\0';
            entries[count].score = atoi(score_str);
            count++;
        } else {
            printf("Error: line format incorrect\n");
        }
    }

    regfree(&regex);

    fclose(file);

    
    bubbleSort(entries, count);

    file = fopen("data.csv", "a+");
    initGameStateFsm(&fsm);
    
    set_noncanonical_mode();
    system("clear");
    printf("Welcome to C-man %s\n",username);
    printf("Play with ONLY W, A, S, D\n");
    printf("Press Enter to start the game...\n");
    printf("Press Spacebar to pause and resume the game...\n");
    printf("Press q to quit the game...\n");
    printf("Scoreboard: \n");
    /* Display high score entries */
    for (i = 0; i < NUM_LINES_TO_READ; i++) {
        if(strcmp(entries[i].username,"")){
         printf("%s: %d\n", entries[i].username, entries[i].score);
        }
    }

    while (fsm.currentGameState != End) {
        /* Read user input */
        if (read(STDIN_FILENO, &input, 1) > 0) {
            processGameState(&fsm, input);
            gameoverprinted = 0;
        }

        /* Display game state */
        if (fsm.currentGameState == Playing) {
            system("clear");
            print_score_board(fsm.scoreboard);
            print_map(fsm.map, fsm.player, fsm.ghosts, fsm.scoreboard);
        } else if (fsm.currentGameState == GameOver){
            handleGameOverState(&fsm,input);
        }

        /* Move ghost periodically */
        if (counter++ >= ghost_move_interval) {
            for (i = 0; i < NUM_GHOSTS; i++) {
                move_ghost(fsm.ghosts[i], fsm.ghosts, fsm.map, get_random_direction());    
            }
            counter = 0;
        }
        handleCManState(&fsm);
        for (i = 0; i < NUM_GHOSTS; i++) {
        /* Check for collision between C-man and ghost, and check if C-man is in POWER_MODE */
            if (check_collision(fsm.player, fsm.ghosts[i])) {
            
                if (fsm.scoreboard->immunity->currentState == NORMAL_MODE && fsm.ghosts[i]->is_active) {
                    
                    if (!gameoverprinted){
                        gameoverprinted = 1;
                        system("clear");
                        print_score_board(fsm.scoreboard);
                        print_map(fsm.map, fsm.player, fsm.ghosts, fsm.scoreboard);
                        printf("\nGame Over! You've been caught by the ghost!\n");
                        printf("Do you want to start over or exit?\nPress 'y' to start over and press 'x' to exit\n");
                        
                    }
                    fsm.currentGameState = GameOver;
                    break;
                } else {
                    fsm.ghosts[i]->is_active = 0;
                    fsm.scoreboard->score += 10;
                }
            }
        }

        if (fsm.currentGameState != GameOver) {
            sleep_microseconds(100000);
        }
       /*sleep_microseconds(100000);*/
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


   



