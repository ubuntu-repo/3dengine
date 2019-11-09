#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

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

typedef struct {
    int a;
    int b;
    int c;
    int face_index;
} triangle;

///////////////////////////////////////////////////////////////////////////////
// Declare cube vertices and triangles
///////////////////////////////////////////////////////////////////////////////
const unsigned int N_CUBE_VERTICES = 8;
point3d cube_vertices[N_CUBE_VERTICES] = {
    { .x = -1, .y = -1, .z =  1},
    { .x = -1, .y =  1, .z =  1},
    { .x =  1, .y =  1, .z =  1},
    { .x =  1, .y = -1, .z =  1},
    { .x = -1, .y = -1, .z = -1},
    { .x = -1, .y =  1, .z = -1},
    { .x =  1, .y =  1, .z = -1},
    { .x =  1, .y = -1, .z = -1}
};

point2d projected_points[N_CUBE_VERTICES];

const unsigned int N_CUBE_TRIANGLES = 6 * 2; // 6 faces, 2 triangles per face
triangle cube_triangles[N_CUBE_TRIANGLES] = {
    // front
    { .a = 0, .b = 1, .c = 2, .face_index =  1 },
    { .a = 2, .b = 3, .c = 0, .face_index =  1 },
    // top
    { .a = 1, .b = 5, .c = 6, .face_index =  2 },
    { .a = 6, .b = 2, .c = 1, .face_index =  2 },
    // back
    { .a = 5, .b = 4, .c = 7, .face_index =  3 },
    { .a = 7, .b = 6, .c = 5, .face_index =  3 },
    // bottom
    { .a = 4, .b = 0, .c = 3, .face_index =  4 },
    { .a = 3, .b = 7, .c = 4, .face_index =  4 },
    // right
    { .a = 3, .b = 2, .c = 6, .face_index =  5 },
    { .a = 6, .b = 7, .c = 3, .face_index =  5 },
    // left
    { .a = 0, .b = 5, .c = 1, .face_index =  6 },
    { .a = 0, .b = 4, .c = 5, .face_index =  6 }
};

///////////////////////////////////////////////////////////////////////////////
// Declare the camera position, rotation, and FOV distortion variables
///////////////////////////////////////////////////////////////////////////////
float fov_factor = 640.0f;
point3d camera_position = { .x = 0, .y = 0, .z = -5 };
point3d cube_rotation = { .x = 0, .y = 0, .z = 0 };

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
// Functions to rotate 3D points in X, Y, and Z
///////////////////////////////////////////////////////////////////////////////
point3d rotate_x(point3d point, float angle) {
    point3d rotated_point = {
        .x = point.x,
        .y = cos(angle) * point.y - sin(angle) * point.z,
        .z = sin(angle) * point.y + cos(angle) * point.z
    };
    return rotated_point;
}

point3d rotate_y(point3d point, float angle) {
    point3d rotated_point = {
        .x = cos(angle) * point.x - sin(angle) * point.z,
        .y = point.y,
        .z = sin(angle) * point.x + cos(angle) * point.z
    };
    return rotated_point;
}

point3d rotate_z(point3d point, float angle) {
    point3d rotated_point = {
        .x = cos(angle) * point.x - sin(angle) * point.y,
        .y = sin(angle) * point.x + cos(angle) * point.y,
        .z = point.z
    };
    return rotated_point;
}

///////////////////////////////////////////////////////////////////////////////
// Function that receives a 3D point and returns a projected 2D point
///////////////////////////////////////////////////////////////////////////////
point2d project(point3d point) {
    point2d projected_point = {
        .x = (fov_factor * (point.x - camera_position.x)) / point.z,
        .y = (fov_factor * (point.y - camera_position.y)) / point.z
    };
    return projected_point;
}

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
            // quit game
            if (event.key.keysym.sym == SDLK_ESCAPE)
                game_is_running = FALSE;
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Setup function to initialize game objects and game state
///////////////////////////////////////////////////////////////////////////////
void setup(void) {
    cube_rotation.x = 0.0;
    cube_rotation.y = 0.0;
    cube_rotation.z = 0.0;
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

    // Loop all cube vertices, rotating and projecting them
    for (int i = 0; i < N_CUBE_VERTICES; i++) {
        point3d point = cube_vertices[i];

        // rotate the original 3d point in the x, y, and z axis
        point3d rotated_point = point;
        rotated_point = rotate_x(rotated_point, cube_rotation.x += 0.05 * delta_time);
        rotated_point = rotate_y(rotated_point, cube_rotation.y += 0.05 * delta_time);
        rotated_point = rotate_z(rotated_point, cube_rotation.z += 0.05 * delta_time);

        // apply the camera transform
        rotated_point.x -= camera_position.x;
        rotated_point.y -= camera_position.y;
        rotated_point.z -= camera_position.z;

        // receives a point3d and returns a point2d projection
        point2d projected_point = project(rotated_point);

        projected_points[i] = projected_point;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Render function to draw game objects in the display
///////////////////////////////////////////////////////////////////////////////
void render(void) {
    // Clear the render background with a black color
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Loop all cube face triangles
    for (int i = 0; i < N_CUBE_TRIANGLES; i++) {
        point2d point_a = projected_points[cube_triangles[i].a];
        point2d point_b = projected_points[cube_triangles[i].b];
        point2d point_c = projected_points[cube_triangles[i].c];

        // Draw a trigon (triangle) from point a, b, and c
        // Also translates/moves them to the center of the screen
        trigonRGBA(
            renderer,
            point_a.x + (window_width / 2), point_a.y + (window_height / 2),
            point_b.x + (window_width / 2), point_b.y + (window_height / 2),
            point_c.x + (window_width / 2), point_c.y + (window_height / 2),
            255, 255, 255, 255
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
