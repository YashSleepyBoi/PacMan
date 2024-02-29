#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

typedef struct {
    int score;
    int level;
} scoreboard;

scoreboard* create_scoreboard(void) {
    scoreboard* sb = malloc(sizeof(scoreboard)); // Allocate memory
    if (sb != NULL) {
        sb->score = 0;
        sb->level = 1;
        printf("%d\n", sb->score);
    }
    return sb;
}

typedef struct{
    int pos_x;
    int pos_y;
} player;

player* create_player(int x, int y){
    player *p = malloc(sizeof(player)); // Allocate memory
    if (p != NULL) {
        p->pos_x = x;
        p->pos_y = y;
    }
    return p;
}

void print_map(int map[3][3]) {
    // Function implementation to print the map
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf("%d ", map[i][j]);
        }
        printf("\n");
    }
}

void set_noncanonical_mode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    term.c_cc[VMIN] = 1; // Set minimum number of characters to read
    term.c_cc[VTIME] = 0; // Set timeout for read (0 seconds)
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

// Function to reset terminal attributes to canonical mode
void set_canonical_mode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= (ICANON | ECHO); // Enable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}



int main() {
    int map[3][3] = {
        {0, 0, 0},
        {0, 1, 0},
        {0, 0, 0}
        };
    player *player = create_player(0, 0);
    set_noncanonical_mode(); // Set non-canonical mode

    char c;
    
    while (1) {
        if (read(STDIN_FILENO, &c, 1) < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        if (c == 'q') {
            set_canonical_mode();
            break;
        }
       
        // printf("%d %d %d\n%d %d %d\n%d %d %d", map[0][0], map[0][1], map[0][2], map[1][0], map[1][1], map[1][2],  map[2][0], map[2][1], map[2][2]);
        map[player->pos_x][player->pos_y] = 7;
      

        if(c == 'a' || c=='U'){
            if(player->pos_y-1>=0){
                
                map[player->pos_x][player->pos_y] = 0;
                player->pos_y--;
                map[player->pos_x][player->pos_y] = 7;
                
            }
        }else if(c == 'd'|| c=='D'){
            if(player->pos_y+1<3){
                map[player->pos_x][player->pos_y] = 0;
                player->pos_y++;
                map[player->pos_x][player->pos_y] = 7;
                
            }
        }else if(c == 'w'|| c=='L'){
            if(player->pos_x -1>=0){
                map[player->pos_x][player->pos_y] = 0;
                player->pos_x--;
                map[player->pos_x][player->pos_y] = 7;
                
            }
        }else if(c == 's'|| c=='R'){
            if(player->pos_x +1<3){
                map[player->pos_x][player->pos_y] = 0;
                player->pos_x++;
                map[player->pos_x][player->pos_y] = 7;
                
            }
        }
        system("clear");
        print_map(map);
    }
    free(player); // Free allocated memory
     return 0;
    }

   



