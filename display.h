#pragma once

#include "game.h"

// Initialize LCD controller and the grlib drawing context.
void LCD_Init(void);

// Draw the current game snapshot without touching shared state.
void DrawGame(const Position *snakeData, uint8_t length, Position food, uint16_t score);
