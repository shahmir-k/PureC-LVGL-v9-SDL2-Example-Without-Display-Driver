#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
//#include "menu.h"
#include "input.h"

#include "lvgl.h"
#include "lv_sdl_private.h"



#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

bool should_exit = false;

// Global variables declarations
SDL_Window* window = NULL; // the SDL window
SDL_Renderer* renderer = NULL; // the SDL renderer
static SDL_Texture* screen_texture = NULL; // the SDL texture for the screen


SDL_GameController* controller = NULL; // the SDL game controller for gamepad input
// Big shoutout to Ruben Wardy for the write up
// https://blog.rubenwardy.com/2023/01/24/using_sdl_gamecontroller/
TTF_Font* font = NULL; // SDL font (not used, should remove)

// LVGL global variables
lv_display_t * disp = NULL; // the LVGL display to render
static uint8_t * buf1 = NULL; // LVGL internal graphics buffer 1
static uint8_t * buf2 = NULL; // LVGL internal graphics buffer 2
static lv_timer_t * event_handler_timer;
typedef struct {
    SDL_Window * window;
    SDL_Renderer * renderer;
    SDL_Texture * texture;
    uint8_t * fb1;
    uint8_t * fb2;
    uint8_t * fb_act;
    uint8_t * buf1;
    uint8_t * buf2;
    uint8_t * rotated_buf;
    size_t rotated_buf_size;
    float zoom;
    uint8_t ignore_size_chg;
} lv_sdl_window_t;
static lv_indev_t *lvMouse;
static lv_indev_t *lvMouseWheel;
static lv_indev_t *lvKeyboard;

// Function prototypes
void cleanup(void); // cleanup function
bool initSDLandLVGL(void); // initialize SDL and LVGL
void handleEvents(lv_timer_t * t); // handle SDL events directly



void releaseLVGLDisplay(lv_event_t * e); // release the LVGL display
static void my_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map); // LVGL flush callback, to flush display and render new frame


bool initSDLandLVGL() {
    
    // Order of operations taken from LVGL internal SDL driver

    /* Initialize LVGL */
    lv_init();



    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    // Initialize SDL_ttf
    if (TTF_Init() < 0) {
        printf("SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
        return false;
    }

    event_handler_timer = lv_timer_create(handleEvents, 5, NULL);
    /* Register SDL tick handler to LVGL */
    lv_tick_set_cb(SDL_GetTicks);
    lv_delay_set_cb(SDL_Delay);

    // Create LVGL display
    disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!disp) {
        printf("Failed to create LVGL display\n");
        return 1;
    }

    // Create our window data structure
    lv_sdl_window_t * dsc = lv_malloc_zeroed(sizeof(lv_sdl_window_t));
    LV_ASSERT_MALLOC(dsc);
    if(dsc == NULL) return false;
    
    // Create SDL window
    window = SDL_CreateWindow("app-exampleLVGL", 
                             SDL_WINDOWPOS_UNDEFINED, 
                             SDL_WINDOWPOS_UNDEFINED, 
                             SCREEN_WIDTH, 
                             SCREEN_HEIGHT, 
                             SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    SDL_SetWindowResizable(window, true);
    
    // Create SDL renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        return false;
    }
    
    // Set blend mode for the renderer
    if (SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND) < 0) {
        printf("Failed to set blend mode: %s\n", SDL_GetError());
        return false;
    }

    // Store window and renderer in our display data
    dsc->window = window;
    dsc->renderer = renderer;
    dsc->zoom = 1.0f;
    lv_display_set_driver_data(disp, dsc);
    printf("Stored window %p (ID: %u) in display %p\n", 
           (void*)window, SDL_GetWindowID(window), (void*)disp);

    //Set renderer clear color to black and clear
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    
    

    lv_display_set_flush_cb(disp, my_flush_cb);
    lv_display_set_default(disp);
    
    /* Allocate render buffers */
    uint32_t buf_size = SCREEN_WIDTH * SCREEN_HEIGHT * 4;  // ARGB8888 = 32 bits = 4 bytes per pixel
    buf1 = malloc(buf_size);
    buf2 = malloc(buf_size);
    
    if (!buf1 || !buf2) {
        printf("Failed to allocate display buffers!\n");
        return false;
    }
    
    /* Initialize display buffers */
    lv_display_set_buffers(disp, buf1, buf2, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
    
    /* Set display color format */
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_ARGB8888);


    
    // Load the game controller database
    bool gameControllerDbFound = false;
    if (!gameControllerDbFound) {
        if (SDL_GameControllerAddMappingsFromFile("/usr/lib/gamecontrollerdb.txt") < 0) {
            printf("Warning: Could not load muOS /usr/lib/gamecontrollerdb.txt: %s\n", SDL_GetError());
            // Don't return false here, we can still function without the mappings
        } else {
            printf("Successfully loaded muOS game controller mappings\n");
            gameControllerDbFound = true;
        }
    }
    if (!gameControllerDbFound) {
        if (SDL_GameControllerAddMappingsFromFile("main/gamecontrollerdb.txt") < 0) {
            printf("Warning: Could not load local gamecontrollerdb.txt: %s\n", SDL_GetError());
            // Don't return false here, we can still function without the mappings
        } else {
            printf("Successfully loaded local game controller mappings\n");
            gameControllerDbFound = true;
        }
    }
    
    // Verify subsystem initialization
    Uint32 subsystems = SDL_WasInit(SDL_INIT_EVERYTHING);
    printf("SDL Subsystems initialized:\n");
    printf("- Video: %s\n", (subsystems & SDL_INIT_VIDEO) ? "Yes" : "No");
    printf("- Joystick: %s\n", (subsystems & SDL_INIT_JOYSTICK) ? "Yes" : "No");
    printf("- GameController: %s\n", (subsystems & SDL_INIT_GAMECONTROLLER) ? "Yes" : "No");
    printf("- Events: %s\n", (subsystems & SDL_INIT_EVENTS) ? "Yes" : "No");
    
    // // Load font from our local fonts directory
    // font = TTF_OpenFont("main/fonts/LiberationSans-Regular.ttf", 24);
    // if (!font) {
    //     printf("Could not load font! TTF_Error: %s\n", TTF_GetError());
    //     return false;
    // }

    // Return true if everything initialized successfully
    return true;
}

/* Display flush callback */
static void my_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map) {
    // Get current display size
    lv_coord_t width = lv_display_get_horizontal_resolution(disp);
    lv_coord_t height = lv_display_get_vertical_resolution(disp);

    // Create or recreate texture if needed
    if (!screen_texture) {
        screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_STREAMING,
                                         width, height);
        if (!screen_texture) {
            printf("Failed to create texture: %s\n", SDL_GetError());
            return;
        }
        SDL_SetTextureBlendMode(screen_texture, SDL_BLENDMODE_BLEND);
    }

    // Lock the texture to get direct access
    void* pixels;
    int pitch;
    SDL_LockTexture(screen_texture, NULL, &pixels, &pitch);

    // Copy the LVGL pixels to the texture
    uint32_t* texture_pixels = (uint32_t*)pixels;
    uint32_t* lvgl_pixels = (uint32_t*)px_map;

    // Copy line by line
    for (int y = area->y1; y <= area->y2; y++) {
        for (int x = area->x1; x <= area->x2; x++) {
            texture_pixels[y * (pitch/4) + x] = lvgl_pixels[(y - area->y1) * (area->x2 - area->x1 + 1) + (x - area->x1)];
        }
    }

    SDL_UnlockTexture(screen_texture);

    if (lv_display_flush_is_last(disp)) {
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    lv_display_flush_ready(disp);
}

void handleEvents(lv_timer_t * t) {
    LV_UNUSED(t);
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Debug output for events
        switch (event.type) {
            
            case SDL_QUIT:
                should_exit = true;
                break;
                
                // case SDL_KEYDOWN:
                //     handleKeyDown(&event);
            //     break;
            
            case SDL_CONTROLLERBUTTONDOWN:
                handleControllerButtonDown(&event);
                break;

                case SDL_CONTROLLERBUTTONUP:
                handleControllerButtonUp(&event);
                break;
                
            case SDL_CONTROLLERAXISMOTION:
                handleControllerAxisMotion(&event);
                break;
                
            case SDL_CONTROLLERDEVICEADDED:
                if (!controller) {
                    controller = SDL_GameControllerOpen(event.cdevice.which);
                    if (controller) {
                        printf("Controller connected: %s\n", SDL_GameControllerName(controller));
                        initInput(controller, cleanup);
                    }
                }
                break;

            case SDL_CONTROLLERDEVICEREMOVED:
                if (controller) {
                    printf("Controller disconnected\n");
                    SDL_GameControllerClose(controller);
                    controller = NULL;
                    // Try to find another controller
                    for (int i = 0; i < SDL_NumJoysticks(); i++) {
                        if (SDL_IsGameController(i)) {
                            controller = SDL_GameControllerOpen(i);
                            if (controller) {
                                printf("Switched to controller: %s\n", SDL_GameControllerName(controller));
                                initInput(controller, cleanup);
                                break;
                            }
                        }
                    }
                }
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED || 
                    event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    int width = event.window.data1;
                    int height = event.window.data2;
                    
                    // Recreate screen texture with new dimensions
                    if (screen_texture) {
                        SDL_DestroyTexture(screen_texture);
                        screen_texture = NULL;
                    }
                    
                    // Update LVGL display size
                    lv_display_set_resolution(disp, width, height);
                    
                    // Reallocate render buffers
                    uint32_t buf_size = width * height * 4;  // ARGB8888 = 4 bytes per pixel
                    uint8_t *new_buf1 = realloc(buf1, buf_size);
                    uint8_t *new_buf2 = realloc(buf2, buf_size);
                    
                    if (new_buf1 && new_buf2) {
                        buf1 = new_buf1;
                        buf2 = new_buf2;
                        lv_display_set_buffers(disp, buf1, buf2, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
                    }
                    
                    // Force a redraw
                    lv_obj_invalidate(lv_scr_act());
                }
                break;
                
            default:
                // Handle input events if devices are available
                if (lvMouse) {
                    lv_sdl_mouse_handler(&event);
                }
                if (lvMouseWheel) {
                    lv_sdl_mousewheel_handler(&event);
                }
                if (lvKeyboard) {
                    lv_sdl_keyboard_handler(&event);
                }
                // Pass all other events to the menu system
                //handleMenuEvents(getCurrentMenu(), &event);
                break;
        }
    }
}

/* LVGL Button event handler */
static void btn_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        printf("\n\n\nButton clicked!\n\n\n");
    }
}

static void keyboard_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        const char* key_str = NULL;
        lv_group_t * g = lv_group_get_default();
        
        // Convert key code to string representation
        switch(key) {
            case LV_KEY_UP: 
                key_str = "UP";
                if(g) lv_group_focus_prev(g);
                break;
            case LV_KEY_DOWN: 
                key_str = "DOWN";
                if(g) lv_group_focus_next(g);
                break;
            case LV_KEY_RIGHT: key_str = "RIGHT"; break;
            case LV_KEY_LEFT: key_str = "LEFT"; break;
            case LV_KEY_ESC:
                key_str = "ESC";
                should_exit = true;
                break;
            case LV_KEY_DEL: key_str = "DEL"; break;
            case LV_KEY_BACKSPACE: key_str = "BACKSPACE"; break;
            case LV_KEY_ENTER: key_str = "ENTER"; break;
            case LV_KEY_NEXT: key_str = "TAB/NEXT"; break;
            case LV_KEY_PREV: key_str = "PREV"; break;
            case LV_KEY_HOME: key_str = "HOME"; break;
            case LV_KEY_END: key_str = "END"; break;
            default: key_str = "OTHER"; break;
        }
        printf("Key pressed: %s (code: %u)\n", key_str, key);
    }
}


void cleanup() {
    printf("Cleaning up...\n");
    
    // First cleanup SDL game controller
    if (controller) {
        SDL_GameControllerClose(controller);
        controller = NULL;
    }

    // Cleanup TTF
    if (font) {
        TTF_CloseFont(font);
        font = NULL;
    }

    // Delete all LVGL objects and screens
    if (lv_scr_act()) {
        lv_obj_clean(lv_scr_act());  // Clean the current screen
        lv_obj_del(lv_scr_act());    // Delete the screen itself
    }
    
    // Cleanup LVGL display
    if (disp) {
        printf("Deleting LVGL display\n");
        lv_display_set_driver_data(disp, NULL);
        lv_display_delete(disp);
        disp = NULL;
    }

    // Cleanup LVGL
    lv_deinit();
    
    // Free display buffers
    if (buf1) {
        free(buf1);  // Using free since we used malloc
        buf1 = NULL;
    }
    if (buf2) {
        free(buf2);  // Using free since we used malloc
        buf2 = NULL;
    }
    
    // Cleanup SDL resources
    if (screen_texture) {
        SDL_DestroyTexture(screen_texture);
        screen_texture = NULL;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }

    // Finally quit SDL subsystems
    TTF_Quit();
    SDL_Quit();
}
bool releasingLVGLDisplay = false;
void releaseLVGLDisplay(lv_event_t * e) {
    if (releasingLVGLDisplay) {
        return;
    }
    releasingLVGLDisplay = true;
    printf("Releasing LVGL display\n");
    LV_UNUSED(e);
    lv_display_set_driver_data(disp, NULL);
    lv_display_delete(disp);
    disp = NULL;
    releasingLVGLDisplay = false;
}

void signalHandler(int sig) {
    LV_UNUSED(sig);
    should_exit = true;
}

int main(int argc, char* argv[]) {
    (void)argc;  // Suppress unused parameter warning
    (void)argv;  // Suppress unused parameter warning
    // Handle program termination signals
    signal(SIGINT, signalHandler);   // Ctrl+C
    signal(SIGTERM, signalHandler);  // Termination request
    signal(SIGHUP, signalHandler);   // Terminal closed
    

    if (!initSDLandLVGL()) {
        printf("Failed to initialize SDL!\n");
        return 1;
    }

    // Create input devices for mouse/keyboard support
    lvMouse = lv_sdl_mouse_create();
    if (lvMouse) {
        printf("Mouse input device created successfully\n");
    }
    
    lvMouseWheel = lv_sdl_mousewheel_create();
    if (lvMouseWheel) {
        printf("Mouse wheel input device created successfully\n");
    }
    
    lvKeyboard = lv_sdl_keyboard_create();
    if (lvKeyboard) {
        printf("Keyboard input device created successfully\n");
    }

    //lv_indev_set_display(lvMouse, disp);  // Associate mouse with our display

    /* Create a screen */
    // lv_obj_t * screen = lv_obj_create(NULL);
    // lv_screen_load(screen);

    // Create and configure group for keyboard navigation
    lv_group_t * g = lv_group_create();
    lv_group_set_default(g);
    lv_indev_set_group(lvKeyboard, g);
    lv_group_set_wrap(g, true);  // Allow wrapping from last to first and vice versa

    // Create a container with flex layout
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 140, LV_SIZE_CONTENT);
    lv_obj_center(cont);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(cont, 10, 0);  // Add 10px gap between buttons

    // Set focus frozen so we can manually focus and unfocus buttons
    lv_group_focus_freeze(g, false);
    
    // Style for focused state
    static lv_style_t style_focus;
    lv_style_init(&style_focus);
    lv_style_set_outline_width(&style_focus, 2);
    lv_style_set_outline_color(&style_focus, lv_color_hex(0x0000ff));  // Blue outline
    lv_style_set_outline_pad(&style_focus, 4);  // Space between outline and button

    // Create first button
    lv_obj_t * btn1 = lv_btn_create(cont);
    lv_obj_set_size(btn1, 120, 50);
    lv_obj_add_event_cb(btn1, btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(btn1, keyboard_event_cb, LV_EVENT_KEY, NULL);
    lv_obj_t * label1 = lv_label_create(btn1);
    lv_label_set_text(label1, "Button 1");
    lv_obj_center(label1);
    lv_obj_add_flag(btn1, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_style(btn1, &style_focus, LV_STATE_FOCUSED | LV_STATE_FOCUS_KEY);
    lv_group_add_obj(g, btn1);

    // Create second button
    lv_obj_t * btn2 = lv_btn_create(cont);
    lv_obj_set_size(btn2, 120, 50);
    lv_obj_add_event_cb(btn2, btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(btn2, keyboard_event_cb, LV_EVENT_KEY, NULL);
    lv_obj_t * label2 = lv_label_create(btn2);
    lv_label_set_text(label2, "Button 2");
    lv_obj_center(label2);
    lv_obj_add_flag(btn2, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_style(btn2, &style_focus, LV_STATE_FOCUSED | LV_STATE_FOCUS_KEY);
    lv_group_add_obj(g, btn2);

    // Focus the first button by default and ensure focus state is applied
    lv_group_focus_obj(btn1);
    
    printf("Detected %d game controllers\n", SDL_NumJoysticks());

    // Initialize game controller
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            controller = SDL_GameControllerOpen(i);
            if (controller) {
                printf("Successfully opened game controller: %s\n", SDL_GameControllerName(controller));
                break;
            } else {
                printf("Failed to open game controller %i: %s\n", i, SDL_GetError());
            }
        } else {
            printf("Controller %i is not a game controller: %s\n", i, SDL_JoystickNameForIndex(i));
        }
    }

    // Initialize the menu system
    //initMenuSystem(renderer, font, cleanup);
    initInput(controller, cleanup);
    
    printf("muOS Screensaver started. Press ESC to go back or exit.\n");
    
    // Main game loop
    while (!should_exit) {
        // Handle SDL events
        //handleEvents(event_handler_timer);
        
        // Let LVGL handle its tasks
        lv_timer_handler();
        
        checkExitCombination();  // Check for exit button combination
        SDL_Delay(10); // Cap at 100 FPS
    }
    
    cleanup();
    return 0;
}
