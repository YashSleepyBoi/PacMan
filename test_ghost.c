#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define WIDTH 20
#define HEIGHT 10

// Define grid symbols
#define WALL '#'
#define EMPTY ' '
#define GHOST 'G'

// Directions
typedef enum { UP, DOWN, LEFT, RIGHT } Direction;

typedef struct {
    int x, y;
} Position;

// Initialize the game grid
void init_grid(char grid[HEIGHT][WIDTH]) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (x == 0 || x == WIDTH - 1 || y == 0 || y == HEIGHT - 1) {
                grid[y][x] = WALL;
            } else {
                grid[y][x] = EMPTY;
            }
        }
    }
}

// Print the grid
void print_grid(char grid[HEIGHT][WIDTH]) {
    system("clear"); // Clear the console (use "cls" on Windows)
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            printf("%c", grid[y][x]);
        }
        printf("\n");
    }
}

// Move the ghost to a new position
void move_ghost(Position *ghost, char grid[HEIGHT][WIDTH], Direction dir) {
    int newX = ghost->x;
    int newY = ghost->y;

    switch (dir) {
        case UP:    newY--; break;
        case DOWN:  newY++; break;
        case LEFT:  newX--; break;
        case RIGHT: newX++; break;
    }

    if (newX >= 0 && newX < WIDTH && newY >= 0 && newY < HEIGHT && grid[newY][newX] != WALL) {
        grid[ghost->y][ghost->x] = EMPTY; // Clear old position
        ghost->x = newX;
        ghost->y = newY;
        grid[ghost->y][ghost->x] = GHOST; // Set new position
    }
}

int main() {
    char grid[HEIGHT][WIDTH];
    Position ghost = { WIDTH / 2, HEIGHT / 2 }; // Start position of the ghost

    srand(time(NULL)); // Seed the random number generator
    init_grid(grid);
    grid[ghost.y][ghost.x] = GHOST; // Place the ghost on the grid

    while (1) {
        print_grid(grid);
        Direction dir = rand() % 4; // Choose a random direction
        move_ghost(&ghost, grid, dir);
        usleep(200000); // Wait for 200ms
    }

    return 0;
}
