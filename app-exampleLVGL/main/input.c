#include "input.h"
#include <stdio.h>
#include <stdlib.h>
#include "lv_sdl_private.h"

static CleanupFunc cleanup = NULL;

// Button state tracking
bool startButtonHeld = false;
bool selectButtonHeld = false;
bool guideButtonHeld = false;
Uint32 buttonHoldStartTime = 0;
const Uint32 BUTTON_HOLD_TIME = 1000; // 1 second in milliseconds

// Controller reference
static SDL_GameController* controller = NULL;

void initInput(SDL_GameController* controllerRef, CleanupFunc cleanupCallback) {
    controller = controllerRef;
    cleanup = cleanupCallback;
}

void handleKeyDown(SDL_Event* event) {
    printf("Keyboard Event - Key: #%d %s\n", event->key.keysym.sym, SDL_GetKeyName(event->key.keysym.sym));
    // Handle power button (1073741926) as escape
    if (event->key.keysym.sym == 1073741926) {
        SDL_Event escEvent;
        SDL_memset(&escEvent, 0, sizeof(SDL_Event));
        escEvent.type = SDL_KEYDOWN;
        escEvent.key.type = SDL_KEYDOWN;
        escEvent.key.state = SDL_PRESSED;
        escEvent.key.keysym.sym = SDLK_ESCAPE;
        //handleMenuEvents(getCurrentMenu(), &escEvent);
        return;
    }
    //handleMenuEvents(getCurrentMenu(), event);
}

void handleControllerButtonDown(SDL_Event* event) {
    printf("Controller Button Down - Button: %d (", event->cbutton.button);

    SDL_Event keyEvent; // create key event to pass to menu system after converting controller input
    SDL_memset(&keyEvent, 0, sizeof(SDL_Event));
    keyEvent.type = SDL_KEYDOWN;
    keyEvent.key.type = SDL_KEYDOWN;
    keyEvent.key.state = SDL_PRESSED;

    switch(event->cbutton.button) {
        case SDL_CONTROLLER_BUTTON_A:
            printf("A");
            keyEvent.key.keysym.sym = SDLK_RETURN;
            //handleMenuEvents(getCurrentMenu(), &keyEvent);
            lv_sdl_keyboard_handler(&keyEvent);
            break;
        case SDL_CONTROLLER_BUTTON_B:
            printf("B");
            keyEvent.key.keysym.sym = SDLK_ESCAPE;
            //handleMenuEvents(getCurrentMenu(), &keyEvent);
            lv_sdl_keyboard_handler(&keyEvent);
            break;
        case SDL_CONTROLLER_BUTTON_X:
            printf("X");
            break;
        case SDL_CONTROLLER_BUTTON_Y:
            printf("Y");
            break;
        case SDL_CONTROLLER_BUTTON_BACK: 
            printf("BACK");
            selectButtonHeld = true;
            break;
        case SDL_CONTROLLER_BUTTON_GUIDE: 
            printf("GUIDE"); 
            guideButtonHeld = true;
            break;
        case SDL_CONTROLLER_BUTTON_START: 
            printf("START"); 
            startButtonHeld = true;
            break;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:
            printf("LEFTSTICK");
            break;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
            printf("RIGHTSTICK");
            break;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            printf("LEFTSHOULDER");
            break;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            printf("RIGHTSHOULDER");
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            printf("DPAD_UP");
            keyEvent.key.keysym.sym = SDLK_UP;
            //handleMenuEvents(getCurrentMenu(), &keyEvent);
            lv_sdl_keyboard_handler(&keyEvent);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            printf("DPAD_DOWN");
            keyEvent.key.keysym.sym = SDLK_DOWN;
            //handleMenuEvents(getCurrentMenu(), &keyEvent);
            lv_sdl_keyboard_handler(&keyEvent);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            printf("DPAD_LEFT");
            keyEvent.key.keysym.sym = SDLK_LEFT;
            //handleMenuEvents(getCurrentMenu(), &keyEvent);
            lv_sdl_keyboard_handler(&keyEvent);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            printf("DPAD_RIGHT");
            keyEvent.key.keysym.sym = SDLK_RIGHT;
            //handleMenuEvents(getCurrentMenu(), &keyEvent);
            lv_sdl_keyboard_handler(&keyEvent);
            break;
        default: printf("UNKNOWN"); break;
    }
    printf(")\n");
}

void handleControllerButtonUp(SDL_Event* event) {
    printf("Controller Button Up - Button: %d\n", event->cbutton.button);
    switch(event->cbutton.button) {
        case SDL_CONTROLLER_BUTTON_BACK:
            selectButtonHeld = false;
            break;
        case SDL_CONTROLLER_BUTTON_GUIDE:
            guideButtonHeld = false;
            break;
        case SDL_CONTROLLER_BUTTON_START:
            startButtonHeld = false;
            break;
    }
}

void handleControllerAxisMotion(SDL_Event* event) {
    // Only print axis events if they pass a threshold to avoid spam
    if (abs(event->caxis.value) > 16384) {  // About 50% of max value
        printf("Controller Axis Event - Axis: %d, Value: %d\n", 
            event->caxis.axis, event->caxis.value);
    }
}

void checkExitCombination() {
    // Check if either combination is held
    bool comboHeld = (startButtonHeld && selectButtonHeld) || 
                    (startButtonHeld && guideButtonHeld);
    
    if (comboHeld) {
        // If this is the start of holding both buttons
        if (buttonHoldStartTime == 0) {
            buttonHoldStartTime = SDL_GetTicks();
            printf("Exit combination detected, hold for 1 second to exit...\n");
        } else {
            // Check if we've held long enough
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime - buttonHoldStartTime >= BUTTON_HOLD_TIME) {
                printf("Exit combination held for 1 second, exiting...\n");
                cleanup();
                exit(0);
            }
        }
    } else {
        // Reset the timer if the combination is broken
        buttonHoldStartTime = 0;
    }
}
