// Minimal Snake (FreeRTOS + GRLIB + Mutex)
// Hardware: TM4C1294XL + Crystalfontz 128x128 LCD

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

extern "C" {
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "sysctl_pll.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
}

// Board drivers (provided in project includes)
#include "button.h"
#include "joystick.h"

// App modules per lab structure
#include "app_objects.h"
#include "game.h"
#include "display.h"

// Shared objects
tContext gContext;
uint32_t gSysClk;
SemaphoreHandle_t gGameStateMutex;

// Buttons used for pause/reset
static Button btnPause(S1);
static Button btnReset(S2);
// Joystick (axes + stick push). Pins from HAL pins.h (BOOSTERPACK1)
static Joystick gJoystick(JSX, JSY, JS1);

// Config
#define INPUT_TICK_MS   10U

// Prototypes
static void configureSystemClock(void);
static void vInputTask(void *pvParameters);
static void vSnakeTask(void *pvParameters);
static void vRenderTask(void *pvParameters);

int main(void)
{
    IntMasterDisable();
    FPUEnable();
    FPULazyStackingEnable();

    configureSystemClock();


    // Init buttons and joystick
    btnPause.begin();
    btnReset.begin();
    gJoystick.begin();
    btnPause.setTickIntervalMs(INPUT_TICK_MS);
    btnReset.setTickIntervalMs(INPUT_TICK_MS);
    gJoystick.setTickIntervalMs(INPUT_TICK_MS);
    btnPause.setDebounceMs(30);
    btnReset.setDebounceMs(30);
    // Optional joystick tuning
    gJoystick.setDeadzone(0.15f);

    gGameStateMutex = xSemaphoreCreateMutex();
    if (gGameStateMutex == NULL) {
        while (1) {
        }
    }

    IntMasterEnable();

    // Create tasks (priorities per lab suggestion)
    xTaskCreate(vInputTask,  "Input",  512, NULL, 2, NULL);
    xTaskCreate(vSnakeTask,  "Snake",  512, NULL, 2, NULL);
    xTaskCreate(vRenderTask, "Render", 768, NULL, 1, NULL);

    vTaskStartScheduler();
    while (1);
}

static void configureSystemClock(void)
{
    gSysClk = SysCtlClockFreqSet(
        SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
        SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480,
        120000000);
}

// Reads joystick/buttons and updates gameState
static void vInputTask(void *pvParameters)
{
    (void)pvParameters;
    Direction requestedDirection;

    for (;;) {
        // Hardware button + joystick polling
        btnPause.tick();
        btnReset.tick();
        gJoystick.tick();

        xSemaphoreTake(gGameStateMutex, portMAX_DELAY); // Lock game state for update shared game data

        // S1 only toggles between Playing and Paused
        if (btnPause.wasPressed()) {
            if (gameState.mode == PLAYING) {
                gameState.mode = PAUSED;
            } else if (gameState.mode == PAUSED) {
                gameState.mode = PLAYING;
            }
        }
        // Pressing S2 restarts the game through ResetGame().
        if (btnReset.wasPressed()) {
            gameState.needsReset = true;
        }

        // Joystick 8-way direction mapping to game directions
        requestedDirection = gameState.currentDirection;
        switch (gJoystick.direction8()) {
            case JoystickDir::N:
                requestedDirection = UP;
                break;
            case JoystickDir::S:
                requestedDirection = DOWN;
                break;
            case JoystickDir::NE:
            case JoystickDir::E:
            case JoystickDir::SE:
                requestedDirection = RIGHT;
                break;
            case JoystickDir::NW:
            case JoystickDir::W:
            case JoystickDir::SW:
                requestedDirection = LEFT;
                break;
            case JoystickDir::Center:
            default:
                // keep last direction
                break;
        }

        if (!((gameState.currentDirection == UP && requestedDirection == DOWN) ||
              (gameState.currentDirection == DOWN && requestedDirection == UP) ||
              (gameState.currentDirection == LEFT && requestedDirection == RIGHT) ||
              (gameState.currentDirection == RIGHT && requestedDirection == LEFT))) {
            gameState.currentDirection = requestedDirection;
        }

        xSemaphoreGive(gGameStateMutex);
        vTaskDelay(pdMS_TO_TICKS(INPUT_TICK_MS));
    }
}

// Advances the snake periodically
static void vSnakeTask(void *pvParameters)
{
    (void)pvParameters;
    xSemaphoreTake(gGameStateMutex, portMAX_DELAY);
    srand((unsigned int)xTaskGetTickCount());
    ResetGame();
    SpawnFood();
    xSemaphoreGive(gGameStateMutex);

    for(;;){
        uint32_t currentTickMs;

        xSemaphoreTake(gGameStateMutex, portMAX_DELAY);
        // take mutex before checking/updatinig shared state through gameState, snake and snakelength
        if (gameState.needsReset) {
            ResetGame();
            SpawnFood();
        }
        if (gameState.mode == PLAYING) {
            moveSnake();
        }
        // reads the current shared tick period under the mutex
        currentTickMs = gSnakeTickMs;
        xSemaphoreGive(gGameStateMutex);
        // now uses that dynamic period to control snake update task
        vTaskDelay(pdMS_TO_TICKS(currentTickMs));
    }
}

// Renders current frame to LCD (guarded by mutex)
static void vRenderTask(void *pvParameters)
{
    (void)pvParameters;
    Position localSnake[MAX_LEN];
    uint8_t localSnakeLength = 0;
    Position localFood;
    uint16_t localScore;
    GameMode localMode;

    LCD_Init();
    TickType_t last = xTaskGetTickCount();
    for(;;)
    {
        xSemaphoreTake(gGameStateMutex, portMAX_DELAY); // Takes mutex before reading snakeLength and snake
        localSnakeLength = snakeLength;
        for (uint8_t i = 0; i < localSnakeLength; ++i) {
            localSnake[i] = snake[i];
        }
        localFood = gFood;  // render cannot read gfood while snake task is updating
        localScore = gScore;
        localMode = gameState.mode;
        xSemaphoreGive(gGameStateMutex);

        DrawGame(localSnake, localSnakeLength, localFood, localScore, localMode);
        vTaskDelayUntil(&last, pdMS_TO_TICKS(33));
    }
}
