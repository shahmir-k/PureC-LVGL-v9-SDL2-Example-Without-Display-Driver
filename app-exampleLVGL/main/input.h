#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>
#include <stdbool.h>

typedef void (*CleanupFunc)(void);

// Button state tracking
extern bool startButtonHeld;
extern bool selectButtonHeld;
extern bool guideButtonHeld;
extern Uint32 buttonHoldStartTime;
extern const Uint32 BUTTON_HOLD_TIME;

// Function declarations

// Initialize with necessary dependencies
void initInput(SDL_GameController* controller, CleanupFunc cleanupCallback);

void handleKeyDown(SDL_Event* event);
void handleControllerButtonDown(SDL_Event* event);
void handleControllerButtonUp(SDL_Event* event);
void handleControllerAxisMotion(SDL_Event* event);

void checkExitCombination(void);

#endif // INPUT_H
