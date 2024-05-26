#include <stdio.h>
#include <vector>
#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/console.h>
#endif

// Define SDL Window and Renderer
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

// Define Screen Width and Height
uint SCREEN_WIDTH = 800;
uint SCREEN_HEIGHT = 600;


void handle_resize(int width, int height) {
    // Set global variables for width and height
    SCREEN_WIDTH = width;
    SCREEN_HEIGHT = height;
    SDL_SetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Logging Screen sizes
    char buffer[50];
    sprintf(buffer, "Resized to Width: %d, Height: %d\n", SCREEN_WIDTH, SCREEN_HEIGHT);
    #ifdef __EMSCRIPTEN__
    emscripten_console_log(buffer);
    #else
    printf("%s", buffer);
    #endif
}


// Create SDL Event handler
static void event_handler(void) {
    // SDL Event Handler
    SDL_Event event;

    // Handle Quit events
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            // Handle emscripten Quit
            #ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
            #else
            // Handle windowed Quit
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            exit(0);
            #endif
        } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
            handle_resize(event.window.data1, event.window.data2);
        }
    }
}

// Create SDL update loop
static void update(void) {
    // call event handler
    event_handler();

    // Clear the renderer with a dark color
    SDL_SetRenderDrawColor(renderer, 40, 42, 54, 255);
    SDL_RenderClear(renderer);

    // Present the renderer
    SDL_RenderPresent(renderer);
}

int main(int argc, char** argv) {
    // Init SDL
    SDL_Init(SDL_INIT_VIDEO);
    // Create SDL Window
    window = SDL_CreateWindow(
        "Particle Simulation", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_RESIZABLE
    );
    // Create SDL Renderer
    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );
    
    // SDL JS Configuration with Emscripten
    #ifdef TEST_SDL_LOCK_OPTS
    EM_ASM({SDL.defaults.copyOnLock = false; SDL.defaults.discardOnLock = true; SDL.defaults.opaqueFront = false;});
    #endif
    
    // Run update function with emscripten (using browsers refresh rate)
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(update, 0, 1);
    #else
    // Run native update loop
    while (1) { update(); }
    #endif

    // returns something
    return 0;
}
