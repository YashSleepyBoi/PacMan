#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>

typedef struct {
    int score;
    int level;
} scoreboard;

scoreboard* create_scoreboard(void) {
    scoreboard* sb = malloc(sizeof(scoreboard)); // Allocate memory
    if (sb != NULL) {
        sb->score = -1;
        sb->level = 1;
    }
    return sb;
}

typedef struct{
    int pos_x;
    int pos_y;
} player;

player* create_player(int x, int y){
    player *player = malloc(sizeof(player)); // Allocate memory
    if (player != NULL) {
        player->pos_x = x;
        player->pos_y = y;
    }
    return player;
}

void set_point_map(int map[10][10]) {
    int i, j;
    int num_big_points = 5;
    srand(time(NULL)); // Initialize random seed

    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
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

void print_map(int map[10][10], player *player) {
    int i, j;
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            if (i == player->pos_x && j == player->pos_y) {
                printf("C "); // Player's position
            } else if (map[i][j] == 1) {
                printf("# "); // Wall
            } else if (map[i][j] == 2) {
                printf(". "); // Small Point
            } else if (map[i][j] == 3) {
                printf("o "); // Big Point
            } else {
                printf("  "); // Empty space
            }
        }
        printf("\n");
    }
}

void print_score_board(scoreboard *scoreboard) {
    printf("LEVEL: %d\n", scoreboard->level);
    printf("SCORE: %d\n", scoreboard->score);
}

void set_noncanonical_mode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    term.c_cc[VMIN] = 0; // Set minimum number of characters to read
    term.c_cc[VTIME] = 1; // Set timeout for read (0 seconds)
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}


// Function to reset terminal attributes to canonical mode
void set_canonical_mode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= (ICANON | ECHO); // Enable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void move_player(player *player, int map[10][10], char direction, scoreboard *scoreboard) {
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

    // Check if the next position is within bounds and not a wall
    if (next_x >= 0 && next_x < 10 && next_y >= 0 && next_y < 10 && map[next_x][next_y] != 1) {
        if (map[next_x][next_y] != 1) { // Not a wall
        // Check if the next position is a dot
            if (map[next_x][next_y] == 2) {
                map[next_x][next_y] = 0; // "Eat" the dot by setting the position to empty
                scoreboard->score++;
            }
            else if (map[next_x][next_y] == 3) {
                map[next_x][next_y] = 0; // "Eat" the dot by setting the position to empty
                scoreboard->score++;
                scoreboard->score++;
            }
        }
            
        // Update the player's position
        player->pos_x = next_x;
        player->pos_y = next_y;
    }
}

int main() {
    int map[10][10] = {
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
    set_point_map(map);
    player *player = create_player(1, 1);
    scoreboard *scoreboard = create_scoreboard();
    set_noncanonical_mode(); // Set non-canonical mode

    system("clear");
    printf("Welcome to C-man\n");
    printf("Play with ONLY W, A, S, D\n");
    printf("Press Spacebar to start the game...\n");
    printf("Press q to quit the game...\n");
    char c, lastDirection = '\0';

    while (1) {
        if (read(STDIN_FILENO, &c, 1) > 0) {
            if (c == 'q') { // to quit the game
                break;
            }
            if (c == 'w' || c == 'a' || c == 's' || c == 'd') {
                lastDirection = c;
            }
        }
        move_player(player, map, lastDirection, scoreboard); // Move player in the last known direction
        system("clear");
        print_score_board(scoreboard);
        print_map(map, player);
     
       
        usleep(10000); // Movement speed
    }

    set_canonical_mode();
    free(player); // Free allocated memory
    free(scoreboard);
    return 0;
}


   



