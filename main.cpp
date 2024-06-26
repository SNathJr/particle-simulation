#include <stdio.h>
#include <vector>
#include <cmath>
#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/console.h>
#endif

// Define SDL Window and Renderer
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

// Define Screen Width and Height
uint SCREEN_WIDTH = 1920;
uint SCREEN_HEIGHT = 960;

// Particles related code
struct Particle {
    float x, y;
    float vx, vy;
    float size = 5.0f;
};

// Gravity and Damping factors
const float GRAVITY = 98.0f; // Exagenrated gravity
const float DAMPING = 0.6f; // Damps collistions
const float FIXED_TIMESTEP = 0.016f; // 16 ms for about 60 fps

// A vector to hold the particles
std::vector<Particle> particles;


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


// Function to create particles on initialization
void init_particles(int num_particles) {
    // clear the particles vector
    particles.clear();
    // compute the center of the screen
    float centerX = SCREEN_WIDTH / 2.0f;
    float centerY = SCREEN_HEIGHT / 2.0f;
    // place the particles around the center of the screen randomly
    for (int i=0; i < num_particles; ++i) {
        Particle p;
        // random positions
        p.x = centerX + rand() % 200 - 100;
        p.y = centerY + rand() % 200 - 100;
        // random velocity
        p.vx = (rand() % 10 - 5) / 1.0f;
        p.vy = (rand() % 10 - 5) / 1.0f;
        // push to vector
        particles.push_back(p);
    }
}


// Function to handle particle collisions
void handle_particle_collisions() {
    for (size_t i=0; i < particles.size(); ++i) {
        for (size_t j=0; j < particles.size(); ++j) {
            // get references to two particles
            Particle& p1 = particles[i];
            Particle& p2 = particles[j];

            // get distance between the particles
            float dx = p2.x - p1.x;
            float dy = p2.y - p1.y;
            float distance = sqrt(dx*dx + dy*dy);
            float min_distance = (p1.size + p2.size) / 2.0f;
            
            // compute collistion among particles
            if (distance < min_distance) {
                // calculate the angle of collision
                float angle = atan2(dy, dx);
                float sin_angle = sin(angle);
                float cos_angle = cos(angle);

                // rotate particle velocities to the coordinate system aligned with the collision angle
                float v1x = p1.vx * cos_angle + p1.vy * sin_angle;
                float v1y = p1.vy * cos_angle - p1.vx * sin_angle;
                float v2x = p2.vx * cos_angle + p2.vy * sin_angle;
                float v2y = p2.vy * cos_angle - p2.vx * sin_angle;

                // exchange the x velocities between p1 and p2
                float temp = v1x;
                v1x = v2x;
                v2x = temp;

                // rotate velocities back to the original coordinate system
                p1.vx = v1x * cos_angle - v1y * sin_angle;
                p1.vy = v1y * cos_angle + v1x * sin_angle;
                p2.vx = v2x * cos_angle - v2y * sin_angle;
                p2.vy = v2y * cos_angle + v2x * sin_angle;

                // separate the particles to prevent overlap
                float overlap = min_distance - distance;
                float separation = overlap / 2.0f;
                p1.x -= separation * cos_angle;
                p1.y -= separation * sin_angle;
                p2.x += separation * cos_angle;
                p2.y += separation * sin_angle;
            }
        }
    }
}


// Function to handle particle simulation
void update_particles(float dt) {
    for (auto& p : particles) {
        // enact gravity on velocity; and velocity on positions (with delta time)
        p.vy += GRAVITY * dt;
        p.x += p.vx * dt;
        p.y += p.vy * dt;

        // handle screen edge collisions on x axis
        if (p.x < 0 || p.x > SCREEN_WIDTH) {
            p.vx = -p.vx * DAMPING;
            p.x = (p.x < 0) ? 0 : SCREEN_WIDTH;
        }

        // handle screen edge collisions on y axis
        if (p.y < 0 || p.y > SCREEN_HEIGHT) {
            p.vy = -p.vy * DAMPING;
            p.y = (p.y < 0) ? 0 : SCREEN_HEIGHT;
        }
    }

    // handle particle collisions
    handle_particle_collisions();
}


// Function to render particles on screen
void render_particles() {
    // set renderer color once
    SDL_SetRenderDrawColor(renderer, 139, 233, 253, 255);
    // draw each particle
    for (const auto& p: particles) {
        SDL_Rect rect = {
            static_cast<int>(p.x - p.size / 2),
            static_cast<int>(p.y - p.size / 2),
            static_cast<int>(p.size),
            static_cast<int>(p.size)
        };
        SDL_RenderFillRect(renderer, &rect);
    }
}


// Create SDL update loop
static void update(void) {
    // create time stamps (static last_time persists out of scope)
    static Uint32 last_time = SDL_GetTicks();
    Uint32 current_time = SDL_GetTicks();
    float elapsed_time = (current_time - last_time) / 1000.0f;
    last_time = current_time;

    // call event handler
    event_handler();

    // update particles with time delta
    while (elapsed_time >= FIXED_TIMESTEP) {
        update_particles(FIXED_TIMESTEP);
        elapsed_time -= FIXED_TIMESTEP;
    }

    // Ensure remaining elapsed time is carried over to the next frame
    if (elapsed_time > 0) {
        update_particles(elapsed_time);
    }

    // Clear the renderer with a dark color
    SDL_SetRenderDrawColor(renderer, 40, 42, 54, 255);
    SDL_RenderClear(renderer);

    // render particles
    render_particles();

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

    // initialize particles
    init_particles(1000);
    
    // SDL JS Configuration with Emscripten
    #ifdef TEST_SDL_LOCK_OPTS
    EM_ASM({SDL.defaults.copyOnLock = false; SDL.defaults.discardOnLock = true; SDL.defaults.opaqueFront = false;});
    #endif

    // Set last time before entering the main loop
    static Uint32 last_time = SDL_GetTicks();
    
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
