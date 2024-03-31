#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>

#define HEIGHT 10
#define WIDTH 10

typedef struct {
    int score;
    int level;
    int immunity;
} scoreboard;

typedef struct {
    int pos_x;
    int pos_y;
    int is_immune;
    int is_active;
} entity;  // Can be used for both player and ghost

scoreboard* create_scoreboard(void) {
    scoreboard* sb = malloc(sizeof(scoreboard));
    if (sb != NULL) {
        sb->score = 0;
        sb->level = 1;
        sb->immunity = 1;
    }
    return sb;
}

entity* create_entity(int x, int y) {
    entity *e = malloc(sizeof(entity));
    if (e != NULL) {
        e->pos_x = x;
        e->pos_y = y;
        e->is_immune = 0;
        e->is_active = 1;
    }
    return e;
}

void set_point_map(int map[HEIGHT][WIDTH]) {
    int i, j;
    int num_big_points = 5;
    srand(time(NULL)); 

    for (i = 0; i < HEIGHT; i++) {
        for (j = 0; j < WIDTH; j++) {
            if (map[i][j] == 0) {
                map[i][j] = 2;
            }
        }
    }
    while (num_big_points > 0) {
        int x = rand() % 10;
        int y = rand() % 10;
        if (map[x][y] == 2) {
            map[x][y] = 3;
            num_big_points--;
        }
    }
}

void print_map(int map[HEIGHT][WIDTH], entity *player, entity *ghost) {
    // Modify to also print the ghost
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
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
            } else {
                printf("  ");
            }
        }
        printf("\n");
    }
}

void print_score_board(scoreboard *scoreboard) {
    printf("LEVEL: %d\n", scoreboard->level);
    printf("SCORE: %d\n", scoreboard->score);
    printf("IMMUNITY: %d\n", scoreboard->immunity);
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

void move_entity(entity *ent, int map[10][10], char direction) {
    int next_x = ent->pos_x;
    int next_y = ent->pos_y;

    switch (direction) {
        case 'w': next_x--; break;
        case 's': next_x++; break;
        case 'a': next_y--; break;
        case 'd': next_y++; break;
    }

    if (next_x >= 0 && next_x < WIDTH && next_y >= 0 && next_y < HEIGHT && map[next_x][next_y] != 1) {
        ent->pos_x = next_x;
        ent->pos_y = next_y;
    }
}

void move_player(entity *player, int map[10][10], char direction, scoreboard *scoreboard) {
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

    if (next_x >= 0 && next_x < 10 && next_y >= 0 && next_y < 10 && map[next_x][next_y] != 1) {
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
    return 's';  // Default to down if something goes wrong
}

int check_collision(entity *player, entity *ghost) {
    return player->pos_x == ghost->pos_x && player->pos_y == ghost->pos_y;
}

int main() {
    int seconds;
    char c, lastDirection;
    entity *player, *ghost;
    scoreboard *scoreboard;
    int map[HEIGHT][WIDTH] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
        {1, 0, 1, 0, 1, 0, 1, 1, 0, 1},
        {1, 0, 1, 0, 0, 0, 0, 1, 0, 1},
        {1, 0, 1, 1, 1, 1, 0, 1, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 1, 1, 0, 1, 1, 1, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    };

    seconds = 0;
    srand(time(NULL));
    set_point_map(map);
    player = create_entity(1, 1); // Starting position of player
    ghost = create_entity(8, 8); // Starting position of ghost
    scoreboard = create_scoreboard();

    set_noncanonical_mode();

    system("clear");
    printf("Welcome to C-man\n");
    printf("Play with ONLY W, A, S, D\n");
    printf("Press Spacebar to start the game...\n");
    printf("Press q to quit the game...\n");
    lastDirection = '\0';

    while (1) {
        if (read(STDIN_FILENO, &c, 1) > 0) {
            if (c == 'q') {
                break;
            }
            if (c == 'w' || c == 'a' || c == 's' || c == 'd') {
                lastDirection = c;
            }
        }

        system("clear");
        print_score_board(scoreboard);
        print_map(map, player, ghost);
        move_player(player, map, lastDirection, scoreboard);
        move_entity(ghost, map, get_random_direction());
        if (check_collision(player, ghost) && ghost->is_active) {
            /*printf("Game Over! The ghost got you!\n");
            break;*/
            if (scoreboard->immunity == 0) {
                printf("Game Over! The ghost got you!\n");
                break;
            } else {
                ghost->is_active = 0;
                scoreboard->score+=10;
                //scoreboard->immunity--;
            }
        }
        seconds++;
        if (seconds > 4 && scoreboard->immunity > 0) {
            scoreboard->immunity--;
            seconds = 0;
        }        
        usleep(200000); // Adjust for game speed
    }

    set_canonical_mode();
    free(player);
    free(ghost);
    free(scoreboard);

    return 0;
}
