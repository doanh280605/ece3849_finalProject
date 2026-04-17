#include <stdint.h>
#include <stdio.h>

extern "C" {
#include "Crystalfontz128x128_ST7735.h"
#include "grlib/grlib.h"
}

#include "app_objects.h"
#include "game.h"

void LCD_Init(void)
{
    Crystalfontz128x128_Init();
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);
    GrContextInit(&gContext, &g_sCrystalfontz128x128);
    GrContextFontSet(&gContext, &g_sFontFixed6x8);

    tRectangle full = {0, 0, 127, 127};
    GrContextForegroundSet(&gContext, ClrBlack);
    GrRectFill(&gContext, &full);
#ifdef GrFlush
    GrFlush(&gContext);
#endif
}

static inline void drawCell(uint8_t gx, uint8_t gy, uint32_t color)
{
    // Convert grid cell to pixel rectangle
    int16_t x0 = (int16_t)(gx * CELL_SIZE);
    int16_t y0 = (int16_t)(gy * CELL_SIZE);
    tRectangle r = { x0, y0, (int16_t)(x0 + CELL_SIZE - 1), (int16_t)(y0 + CELL_SIZE - 1) };
    GrContextForegroundSet(&gContext, color);
    GrRectFill(&gContext, &r);
}


void DrawGame(const Position *snakeData, uint8_t length, Position food, uint16_t score, GameMode mode)
{
    char scoreText[20];
    char finalScoreText[24];
    char pausedScoreText[24];

    // Clear background
    tRectangle full = {0, 0, 127, 127};
    GrContextForegroundSet(&gContext, (mode == GAME_OVER || mode == PAUSED) ? ClrBlue : ClrBlack);
    GrRectFill(&gContext, &full);

    if (mode == GAME_OVER) {
        snprintf(finalScoreText, sizeof(finalScoreText), "Final Score: %u", score);
        GrContextForegroundSet(&gContext, ClrYellow);
        GrStringDraw(&gContext, (char *)"GAME OVER", -1, 40, 42, false);
        GrStringDraw(&gContext, finalScoreText, -1, 25, 58, false);
        GrStringDraw(&gContext, (char *)"Press S2 to play again", -1, 0, 74, false);
#ifdef GrFlush
        GrFlush(&gContext);
#endif
        return;
    }

    if (mode == PAUSED) {
        snprintf(pausedScoreText, sizeof(pausedScoreText), "Score: %u", score);
        GrContextForegroundSet(&gContext, ClrYellow);
        GrStringDraw(&gContext, (char *)"PAUSED", -1, 45, 42, false);
        GrStringDraw(&gContext, pausedScoreText, -1, 40, 58, false);
        GrStringDraw(&gContext, (char *)"Press S1 to continue", -1, 5, 74, false);
#ifdef GrFlush
        GrFlush(&gContext);
#endif
        return;
    }

    snprintf(scoreText, sizeof(scoreText), "Score: %u", score);
    GrContextForegroundSet(&gContext, ClrWhite);
    GrStringDraw(&gContext, scoreText, -1, 2, 2, false);

    drawCell(food.x, food.y, ClrRed);

    // Draw snake
    for (uint8_t i = 0; i < length; ++i) {
        drawCell(snakeData[i].x, snakeData[i].y, i == 0 ? ClrGreen : ClrYellow);
    }

#ifdef GrFlush
    GrFlush(&gContext);
#endif
}
