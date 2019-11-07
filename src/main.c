#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "./constants.h"

///////////////////////////////////////////////////////////////////////////////
// Define constants for FPS and game loop frame time
///////////////////////////////////////////////////////////////////////////////
#define FPS 30
#define FRAME_TARGET_TIME (1000 / FPS)

///////////////////////////////////////////////////////////////////////////////
// Declare a new enumeration type for Boolean variables
///////////////////////////////////////////////////////////////////////////////
typedef enum { FALSE, TRUE } bool;

///////////////////////////////////////////////////////////////////////////////
// Struct declaration for 2D and 3D Points
///////////////////////////////////////////////////////////////////////////////
typedef struct {
    float x;
    float y;
} point2d;

typedef struct {
    float x;
    float y;
    float z;
} point3d;

///////////////////////////////////////////////////////////////////////////////
// Declare an array of points
///////////////////////////////////////////////////////////////////////////////
const int N_POINTS = 9 * 9 * 9; // 9 x 9 x 9 cube
point3d cube_points[N_POINTS];

///////////////////////////////////////////////////////////////////////////////
// Declare the camera position, rotation, and FOV distortion variables
///////////////////////////////////////////////////////////////////////////////
float camera_distortion = 640.0f;
point3d camera_position = { .x = 0, .y = 0, .z = -5 };
point3d camera_rotation = { .x = 0, .y = 0, .z = 0 };

///////////////////////////////////////////////////////////////////////////////
// Global variables for SDL Window, Renderer, and game status
///////////////////////////////////////////////////////////////////////////////
bool game_is_running = FALSE;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
unsigned int last_frame_time = 0;
unsigned window_width = 800;
unsigned window_height = 600;

///////////////////////////////////////////////////////////////////////////////
// Function to initialize the SDL Window and Renderer
///////////////////////////////////////////////////////////////////////////////
int initialize_window(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return FALSE;
    }

    // Set width and height of the SDL Window with the max screen resolution
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);
    window_width = display_mode.w;
    window_height = display_mode.h;

    window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        SDL_WINDOW_BORDERLESS
    );
    if (!window) {
        fprintf(stderr, "Error creating SDL Window.\n");
        return FALSE;
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error creating SDL Renderer.\n");
        return FALSE;
    }

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// Poll system events and handle keyboard presses
///////////////////////////////////////////////////////////////////////////////
void process_input(void) {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
        case SDL_QUIT:
            game_is_running = FALSE;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
                game_is_running = FALSE;
            if (event.key.keysym.sym == SDLK_LEFT)
                camera_rotation.y += 0.03;
            if (event.key.keysym.sym == SDLK_RIGHT)
                camera_rotation.y -= 0.03;
            if (event.key.keysym.sym == SDLK_UP)
                camera_rotation.x += 0.03;
            if (event.key.keysym.sym == SDLK_DOWN)
                camera_rotation.x -= 0.03;
            if (event.key.keysym.sym == SDLK_a)
                camera_distortion += 10.0;
            if (event.key.keysym.sym == SDLK_s)
                camera_position.z -= 0.1;
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Setup function to initialize game objects and game state
///////////////////////////////////////////////////////////////////////////////
void setup(void) {
    int point_count = 0;

    // The cube goes from -1 to 1
    // That means that our cube has edges with length 2
    // 9 x 9 x 9 cube
    float increment_step = 0.25;
    for (float x = -1; x <= 1; x += increment_step) {
        for (float y = -1; y <= 1; y += increment_step) {
            for (float z = -1; z <= 1; z += increment_step) {
                point3d new_point = { .x = x, .y = y, .z = z };
                cube_points[point_count] = new_point;
                point_count++;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Update function with a fixed time step
///////////////////////////////////////////////////////////////////////////////
void update(void) {
    // Waste some time / sleep until we reach the frame target time
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), last_frame_time + FRAME_TARGET_TIME));

    // Get a delta time factor converted to seconds to be used to update my objects
    float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;

    // Store the milliseconds of the current frame
    last_frame_time = SDL_GetTicks();

    // TODO: Update game objects
}

///////////////////////////////////////////////////////////////////////////////
// Functions to render points, lines, and triangles
///////////////////////////////////////////////////////////////////////////////
void render_point(int x, int y, int width, int height) {
    SDL_Rect point = {x, y, width, height};
    SDL_RenderFillRect(renderer, &point);
}

void render_line(int x0, int y0, int x1, int y1) {
    SDL_RenderDrawLine(renderer, x0, y0, x1, y1);
}

void render_triangle(int x0, int y0, int x1, int y1, int x2, int y2) {
    render_line(x0, y0, x1, y1);
    render_line(x1, y1, x2, y2);
    render_line(x2, y2, x0, y0);
}

///////////////////////////////////////////////////////////////////////////////
// Function that receives a 3D point and returns a projected 2D point
///////////////////////////////////////////////////////////////////////////////
point2d project(point3d point) {
    point2d projected_point = {
        .x = (camera_distortion * (point.x - camera_position.x)) / point.z,
        .y = (camera_distortion * (point.y - camera_position.y)) / point.z
    };
    return projected_point;
}

///////////////////////////////////////////////////////////////////////////////
// Functions to rotate 3D points in X, Y, and Z
///////////////////////////////////////////////////////////////////////////////
point3d rotate_y(point3d point, float angle) {
    point3d rotated_point = {
        .x = cos(angle) * point.x - sin(angle) * point.z,
        .y = point.y,
        .z = sin(angle) * point.x + cos(angle) * point.z
    };
    return rotated_point;
}

point3d rotate_x(point3d point, float angle) {
    point3d rotated_point = {
        .x = point.x,
        .y = cos(angle) * point.y - sin(angle) * point.z,
        .z = sin(angle) * point.y + cos(angle) * point.z
    };
    return rotated_point;
}

///////////////////////////////////////////////////////////////////////////////
// Render function to draw game objects in the display
///////////////////////////////////////////////////////////////////////////////
void render(void) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (int i = 0; i < N_POINTS; i++) {
        point3d point = cube_points[i];

        // rotate the original 3d point in the x and y axis
        point3d rotated_point = rotate_y(rotate_x(point, camera_rotation.x), camera_rotation.y);

        rotated_point.x -= camera_position.x;
        rotated_point.y -= camera_position.y;
        rotated_point.z -= camera_position.z;

        // receives a point3d and returns a point2d projection
        point2d projected_point = project(rotated_point);

        render_point(
            projected_point.x + window_width / 2,
            projected_point.y + window_height / 2,
            8 - (rotated_point.z * 1.2), // closer points appear bigger
            8 - (rotated_point.z * 1.2)  // closer points appear bigger
        );
    }

    SDL_RenderPresent(renderer);
}

///////////////////////////////////////////////////////////////////////////////
// Function to destroy renderer, window, and exit SDL
///////////////////////////////////////////////////////////////////////////////
void destroy_window(void) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

///////////////////////////////////////////////////////////////////////////////
// Main function
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {
    game_is_running = initialize_window();

    setup();

    while (game_is_running) {
        process_input();
        update();
        render();
    }

    destroy_window();

    return 0;
}
