#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <SDL2/SDL.h>

///////////////////////////////////////////////////////////////////////////////
// Define two macros to return the max and min between two values
///////////////////////////////////////////////////////////////////////////////
#define MAX(x,y) ((x) >= (y)) ? (x) : (y)
#define MIN(x,y) ((x) <= (y)) ? (x) : (y)

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
    uint32_t color;
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
    { .a = 0, .b = 1, .c = 2, .face_index =  1, .color = 0xFFFF0000 },
    { .a = 2, .b = 3, .c = 0, .face_index =  1, .color = 0xFFFF0000 },
    // top
    { .a = 1, .b = 5, .c = 6, .face_index =  2, .color = 0xFF00FF00 },
    { .a = 6, .b = 2, .c = 1, .face_index =  2, .color = 0xFF00FF00 },
    // back
    { .a = 5, .b = 4, .c = 7, .face_index =  3, .color = 0xFF0000FF },
    { .a = 7, .b = 6, .c = 5, .face_index =  3, .color = 0xFF0000FF },
    // bottom
    { .a = 4, .b = 0, .c = 3, .face_index =  4, .color = 0xFFFFFF00 },
    { .a = 3, .b = 7, .c = 4, .face_index =  4, .color = 0xFFFFFF00 },
    // right
    { .a = 3, .b = 2, .c = 6, .face_index =  5, .color = 0xFF00FFFF },
    { .a = 6, .b = 7, .c = 3, .face_index =  5, .color = 0xFF00FFFF },
    // left
    { .a = 0, .b = 5, .c = 1, .face_index =  6, .color = 0xFFFFFFFF },
    { .a = 0, .b = 4, .c = 5, .face_index =  6, .color = 0xFFFFFFFF }
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
// Declare a color buffer array and a texture that will be used to display it
///////////////////////////////////////////////////////////////////////////////
uint32_t* color_buffer = NULL;
SDL_Texture* color_buffer_texture;

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
    color_buffer = (uint32_t *) malloc(sizeof(uint32_t) * (uint32_t)window_width * (uint32_t) window_height);

    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );
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
// Renders the color buffer array in a texture and displays it
///////////////////////////////////////////////////////////////////////////////
void render_color_buffer() {
    SDL_UpdateTexture(color_buffer_texture, NULL, color_buffer, (int)((uint32_t)window_width * sizeof(uint32_t)));
    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

void clear_color_buffer(uint32_t color) {
    for (int x = 0; x < window_width; x++)
        for (int y = 0; y < window_height; y++)
            color_buffer[(window_width * y) + x] = color;
}

///////////////////////////////////////////////////////////////////////////////
// Render drawing primitives
///////////////////////////////////////////////////////////////////////////////
void draw_pixel(int x, int y, uint32_t color) {
    color_buffer[(window_width * y) + x] = color;
}

///////////////////////////////////////////////////////////////////////////////
// Render a line using the inneficient DDA line drawing algorithm
///////////////////////////////////////////////////////////////////////////////
void dda_line(int x1, int y1, int x2, int y2, uint32_t color) {
    int delta_x = (x2 - x1);
    int delta_y = (y2 - y1);

    int step = abs(delta_x) >= abs(delta_y) ? abs(delta_x) : abs(delta_y);

    // Algorithm is naive and slow as it uses floating point for step increment
    float x_inc = delta_x / (float) step;
    float y_inc = delta_y / (float) step;

    float x = x1;
    float y = y1;
    for (int i = 0; i <= step; i++) {
        draw_pixel((int)x, (int)y, color);
        x += x_inc;
        y += y_inc;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Render a line using the Bresenham line drawing algorithm
///////////////////////////////////////////////////////////////////////////////
void plot_line_low(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = (x1 - x0);
    int dy = (y1 - y0);
    int yi = 1;

    if (dy < 0) {
        yi = -1;
        dy = -dy;
    }

    int D = (2 * dy) - dx;
    int y = y0;

    for (int x = x0; x <= x1; ++x) {
        draw_pixel(x, y, color);
        if (D > 0) {
            y = y + yi;
            D = D - (2 * dx);
        }
        D = D + 2*dy;
    }
}

void plot_line_high(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = (x1 - x0);
    int dy = (y1 - y0);
    int xi = 1;

    if (dx < 0) {
        xi = -1;
        dx = -dx;
    }

    int D = (2 * dx) - dy;
    int x = x0;

    for (int y = y0; y <= y1; ++y) {
        draw_pixel(x, y, color);
        if (D > 0) {
            x = x + xi;
            D = D - (2 * dy);
        }
        D = D + (2 * dx);
    }
}

void bresenham_line(int x0, int y0, int x1, int y1, uint32_t color) {
    if (abs(y1 - y0) < abs(x1 - x0)) {
        if (x0 > x1)
            plot_line_low(x1, y1, x0, y0, color);
        else
            plot_line_low(x0, y0, x1, y1, color);
    } else {
        if (y0 > y1)
            plot_line_high(x1, y1, x0, y0, color);
        else
            plot_line_high(x0, y0, x1, y1, color);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Draw an unfilled triangle using raw line calls
///////////////////////////////////////////////////////////////////////////////
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    bresenham_line(x0, y0, x1, y1, color);
    bresenham_line(x1, y1, x2, y2, color);
    bresenham_line(x2, y2, x0, y0, color);
}

///////////////////////////////////////////////////////////////////////////////
// Perpendicular dot product between two 2D vectors
///////////////////////////////////////////////////////////////////////////////
float cross_z(point2d a, point2d b, point2d c) {
    // flip y because of the coordinate system
    return (b.x - a.x) * -(c.y - a.y) - -(b.y - a.y) * (c.x - a.x);
}

///////////////////////////////////////////////////////////////////////////////
// Draw an unfilled triangle using raw line calls
///////////////////////////////////////////////////////////////////////////////
void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    // get the bounding box of the triangle
    int max_x = MAX(x0, MAX(x1, x2));
    int min_x = MIN(x0, MIN(x1, x2));
    int max_y = MAX(y0, MAX(y1, y2));
    int min_y = MIN(y0, MIN(y1, y2));

    point2d v0 = { .x = x0, .y = y0 };
    point2d v1 = { .x = x1, .y = y1 };
    point2d v2 = { .x = x2, .y = y2 };

    for (int x = min_x; x < max_x; x++) {
        for (int y = min_y; y < max_y; y++) {
            // sample from the center of the pixel, not the top-left corner
            point2d p = { .x = x + 0.5, .y = y + 0.5 };
            // if the point is not inside our polygon, skip fragment
            if (cross_z(v1, v2, p) < 0 || cross_z(v2, v0, p) < 0 || cross_z(v0, v1, p) < 0) {
                continue;
            } else {
                draw_pixel(x, y, color);
            }
        }
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
        uint32_t triangle_color = cube_triangles[i].color;

        // Draw a filled triangle and translates it to the screen center
        draw_filled_triangle(
            point_a.x + (window_width / 2),
            point_a.y + (window_height / 2),
            point_b.x + (window_width / 2),
            point_b.y + (window_height / 2),
            point_c.x + (window_width / 2),
            point_c.y + (window_height / 2),
            triangle_color
        );

        // Draw triangle lines and translate them to the screen center
        draw_triangle(
            point_a.x + (window_width / 2),
            point_a.y + (window_height / 2),
            point_b.x + (window_width / 2),
            point_b.y + (window_height / 2),
            point_c.x + (window_width / 2),
            point_c.y + (window_height / 2),
            0x00000000
        );
    }

    // Render the color buffer using a SDL texture
    render_color_buffer();

    // Clear the colorBuffer before the reder of the next frame
    clear_color_buffer(0xFF000000);

    SDL_RenderPresent(renderer);
}

///////////////////////////////////////////////////////////////////////////////
// Function to destroy renderer, window, and exit SDL
///////////////////////////////////////////////////////////////////////////////
void destroy_window(void) {
    free(color_buffer);
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
