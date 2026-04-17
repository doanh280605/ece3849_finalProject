#include "game.h"

#include <stdlib.h>

// Global state definitions
SnakeGameState gameState = { RIGHT, PLAYING, false };

Position snake[MAX_LEN];
uint8_t snakeLength = 4; // Start with a snake length of 4
Position gFood;
uint16_t gScore = 0;
uint32_t gSnakeTickMs = SNAKE_TICK_START_MS;

// If the coordinate goes below 0, move it to GRID_SIZE - 1.
// If the coordinate reaches GRID_SIZE, move it back to 0.
// Notes:
//  - We use uint8_t; subtracting 1 from 0 would underflow to 255.
//    To avoid underflow side effects, we check bounds before increment/decrement.

void ResetGame(void)
{
    // Place snake centered, heading right
    snakeLength = 4;
    uint8_t cx = GRID_SIZE / 2;
    uint8_t cy = GRID_SIZE / 2;
    // Head at [cx, cy], body to the left
    for (uint8_t i = 0; i < snakeLength; ++i) {
        snake[i].x = (uint8_t)(cx - i);
        snake[i].y = cy;
    }
    gameState.currentDirection = RIGHT;
    gameState.mode = PLAYING;
    gameState.needsReset = false;
    gScore = 0;
    gSnakeTickMs = SNAKE_TICK_START_MS;
}

// loops until it finds a random cell that does not overlap the current snake 
void SpawnFood(void)
{
    bool overlapsSnake;

    do {
        overlapsSnake = false;
        gFood.x = (uint8_t)(rand() % GRID_SIZE);
        gFood.y = (uint8_t)(rand() % GRID_SIZE);

        for (uint8_t i = 0; i < snakeLength; ++i) {
            if (snake[i].x == gFood.x && snake[i].y == gFood.y) {
                overlapsSnake = true;
                break;
            }
        }
    } while (overlapsSnake);
}

void moveSnake()
{
    // Shift body so each segment follows the previous one
    for (uint8_t i = snakeLength; i > 0; i--) {
        snake[i] = snake[i - 1];
    }

    // Update head position based on direction with  wrap-around.
    switch (gameState.currentDirection) {
        case UP:
            // If at the top and moving up => wrap to bottom
            if (snake[0].y == 0) {
                snake[0].y = (uint8_t)(GRID_SIZE - 1);
            } else {
                snake[0].y = (uint8_t)(snake[0].y - 1);
            }
            break;
        case DOWN:
            // If at the bottom and moving down => wrap to top
            if (snake[0].y == GRID_SIZE - 1) {
                snake[0].y = 0;
            } else {
                snake[0].y = (uint8_t)(snake[0].y + 1);
            }
            break;
        case LEFT:
            if (snake[0].x == 0) {
                snake[0].x = (uint8_t)(GRID_SIZE - 1);
            } else {
                snake[0].x = (uint8_t)(snake[0].x - 1);
            }
            break;
        case RIGHT:
            if (snake[0].x == GRID_SIZE - 1) {
                snake[0].x = 0;
            } else {
                snake[0].x = (uint8_t)(snake[0].x + 1);
            }
            break;
    }

    // the food check happens after movement, because you only know whether the head reached food after the head is 
    // in its new position
    if (snake[0].x == gFood.x && snake[0].y == gFood.y) {
        if (snakeLength < MAX_LEN) {
            ++snakeLength;
        }
        ++gScore;
        if (gSnakeTickMs > SNAKE_TICK_MIN_MS) {
            // the period is clamped so it never goes below 60ms
            if ((gSnakeTickMs - SNAKE_TICK_STEP_MS) < SNAKE_TICK_MIN_MS) {
                gSnakeTickMs = SNAKE_TICK_MIN_MS;
            } else {
                gSnakeTickMs -= SNAKE_TICK_STEP_MS;
            }
        }
        SpawnFood();
    }

    for (uint8_t i = 1; i < snakeLength; ++i) {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
            gameState.mode = GAME_OVER;
            break;
        }
    }
}
