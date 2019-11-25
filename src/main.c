#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

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
} vector2d;

typedef struct {
    float x;
    float y;
    float z;
} vector3d;

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
const unsigned int N_VERTICES = 8;
vector3d vertex_list[N_VERTICES] = {
    { .x = -1, .y = -1, .z =  1},
    { .x = -1, .y =  1, .z =  1},
    { .x =  1, .y =  1, .z =  1},
    { .x =  1, .y = -1, .z =  1},
    { .x = -1, .y = -1, .z = -1},
    { .x = -1, .y =  1, .z = -1},
    { .x =  1, .y =  1, .z = -1},
    { .x =  1, .y = -1, .z = -1}
};

const unsigned int N_TRIANGLES = 6 * 2; // 6 faces, 2 triangles per face
triangle triangle_list[N_TRIANGLES] = {
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
// Array of updated vertices, triangle faces, and vertex depth values
///////////////////////////////////////////////////////////////////////////////
vector2d projected_points[N_VERTICES];
float vertex_depth_list[N_VERTICES];
vector3d working_vertex_list[N_VERTICES];

///////////////////////////////////////////////////////////////////////////////
// Declare the camera position, rotation, and FOV distortion variables
///////////////////////////////////////////////////////////////////////////////
float fov_factor = 640.0f;
vector3d camera_position = { .x = 0, .y = 0, .z = 0 };
vector3d cube_position = { .x = 0, .y = 0, .z = -5 };
vector3d cube_rotation = { .x = 0, .y = 0, .z = 0 };

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
vector3d rotate_x(vector3d point, float angle) {
    vector3d working_vertex = {
        .x = point.x,
        .y = cos(angle) * point.y - sin(angle) * point.z,
        .z = sin(angle) * point.y + cos(angle) * point.z
    };
    return working_vertex;
}

vector3d rotate_y(vector3d point, float angle) {
    vector3d working_vertex = {
        .x = cos(angle) * point.x - sin(angle) * point.z,
        .y = point.y,
        .z = sin(angle) * point.x + cos(angle) * point.z
    };
    return working_vertex;
}

vector3d rotate_z(vector3d point, float angle) {
    vector3d working_vertex = {
        .x = cos(angle) * point.x - sin(angle) * point.y,
        .y = sin(angle) * point.x + cos(angle) * point.y,
        .z = point.z
    };
    return working_vertex;
}

///////////////////////////////////////////////////////////////////////////////
// Function that receives a 3D point and returns a projected 2D point
///////////////////////////////////////////////////////////////////////////////
vector2d project(vector3d point) {
    vector2d projected_point = {
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
// Renders the color buffer array in a texture and displays it
///////////////////////////////////////////////////////////////////////////////
void render_color_buffer() {
    SDL_UpdateTexture(color_buffer_texture, NULL, color_buffer, (int)((uint32_t)window_width * sizeof(uint32_t)));
    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

///////////////////////////////////////////////////////////////////////////////
// Routine to clear the entire color buffer with a single color value
///////////////////////////////////////////////////////////////////////////////
void clear_color_buffer(uint32_t color) {
    for (int x = 0; x < window_width; x++)
        for (int y = 0; y < window_height; y++)
            color_buffer[(window_width * y) + x] = color;
}

///////////////////////////////////////////////////////////////////////////////
// Set a pixel with a given colour
///////////////////////////////////////////////////////////////////////////////
void draw_pixel(int x, int y, uint32_t color) {
    color_buffer[(window_width * y) + x] = color;
}

///////////////////////////////////////////////////////////////////////////////
// Render a line using the inneficient DDA line drawing algorithm
///////////////////////////////////////////////////////////////////////////////
void draw_line(int x1, int y1, int x2, int y2, uint32_t color) {
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
// Draw an unfilled triangle using raw line calls
///////////////////////////////////////////////////////////////////////////////
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color);
}

////////////////////////////////////////////////////////////////////////////////
// Normalize a vector
////////////////////////////////////////////////////////////////////////////////
void normalize(vector3d* vector) {
    float length = sqrt(
        (vector->x * vector->x) +
        (vector->y * vector->y) +
        (vector->z * vector->z)
    );
    vector->x /= length;
    vector->y /= length;
    vector->z /= length;
}

////////////////////////////////////////////////////////////////////////////////
// Dot product between two vectors
////////////////////////////////////////////////////////////////////////////////
uint32_t apply_light(uint32_t color, float percentage_factor) {
    uint32_t a = (color & 0xFF000000);
    uint32_t r = (color & 0x00FF0000) * percentage_factor;
    uint32_t g = (color & 0x0000FF00) * percentage_factor;
    uint32_t b = (color & 0x000000FF) * percentage_factor;
    uint32_t new_color = a | (r & 0x00FF0000) | (g & 0x0000FF00) | (b & 0x000000FF);
    return new_color;
}

////////////////////////////////////////////////////////////////////////////////
// Dot product between two vectors
////////////////////////////////////////////////////////////////////////////////
float dot(vector3d a, vector3d b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

////////////////////////////////////////////////////////////////////////////////
// Returns true if vertices are in clockwise order
////////////////////////////////////////////////////////////////////////////////
bool is_cw(vector2d a, vector2d b, vector2d c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x) >= 0;
}

////////////////////////////////////////////////////////////////////////////////
// Perpendicular dot product between two 2D vectors
///////////////////////////////////////////////////////////////////////////////
float cross_z(vector2d a, vector2d b, vector2d c, bool invert_cw_points) {
    if (invert_cw_points)
        return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    else
        return (b.x - a.x) * -(c.y - a.y) - -(b.y - a.y) * (c.x - a.x);
}

///////////////////////////////////////////////////////////////////////////////
// Draw a filled triangle
///////////////////////////////////////////////////////////////////////////////
void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    // get the bounding box of the triangle
    int max_x = MAX(x0, MAX(x1, x2));
    int min_x = MIN(x0, MIN(x1, x2));
    int max_y = MAX(y0, MAX(y1, y2));
    int min_y = MIN(y0, MIN(y1, y2));

    vector2d v0 = { .x = x0, .y = y0 };
    vector2d v1 = { .x = x1, .y = y1 };
    vector2d v2 = { .x = x2, .y = y2 };

    bool cw = is_cw(v0, v1, v2);

    for (int x = min_x; x < max_x; x++) {
        for (int y = min_y; y < max_y; y++) {
            // sample from the center of the pixel, not the top-left corner
            vector2d p = { .x = x + 0.5, .y = y + 0.5 };

            // if the point is not inside our polygon, skip fragment
            if (cross_z(v1, v2, p, cw) < 0 || cross_z(v2, v0, p, cw) < 0 || cross_z(v0, v1, p, cw) < 0)
                continue;
            else
                draw_pixel(x, y, color);
        }
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
    for (int i = 0; i < N_VERTICES; i++) {
        vector3d current_point = vertex_list[i];

        // rotate the original 3d point in the x, y, and z axis
        vector3d working_vertex = current_point;
        working_vertex = rotate_x(working_vertex, cube_rotation.x += 0.03 * delta_time);
        working_vertex = rotate_y(working_vertex, cube_rotation.y += 0.05 * delta_time);
        working_vertex = rotate_z(working_vertex, cube_rotation.z += 0.07 * delta_time);

        // apply the camera transform
        working_vertex.x -= cube_position.x;
        working_vertex.y -= cube_position.y;
        working_vertex.z -= cube_position.z;

        // receives a vector3d and returns a vector2d projection
        vector2d projected_point = project(working_vertex);

        // save the 2d projected points
        projected_points[i] = projected_point;

        // save rotated vertex in a list
        working_vertex_list[i] = working_vertex;

        // save the depth of all vertices
        vertex_depth_list[i] = working_vertex.z;
    }

    // calculate the average z-depth of each triangle
    float average_depth_list[N_TRIANGLES];
    for (int i = 0; i < N_TRIANGLES; i++) {
        average_depth_list[i] = vertex_depth_list[triangle_list[i].a];
        average_depth_list[i] += vertex_depth_list[triangle_list[i].b];
        average_depth_list[i] += vertex_depth_list[triangle_list[i].c];
        average_depth_list[i] /= 3.0;
    }

    // sort triangles by their average depth value
    for (int i = 0; i < N_TRIANGLES; i++) {
        for (int j = 0; j < N_TRIANGLES - 1; j++) {
            if (average_depth_list[i] > average_depth_list[j]) {
                // swap the triangls in the original triangle list
                triangle temp_triangle = triangle_list[i];
                triangle_list[i] = triangle_list[j];
                triangle_list[j] = temp_triangle;
                // also swap the depth value in the depth array
                float temp_depth = average_depth_list[i];
                average_depth_list[i] = average_depth_list[j];
                average_depth_list[j] = temp_depth;
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

    // Loop all cube face triangles to render them one by one
    for (int i = 0; i < N_TRIANGLES; i++) {
        vector2d point_a = projected_points[triangle_list[i].a];
        vector2d point_b = projected_points[triangle_list[i].b];
        vector2d point_c = projected_points[triangle_list[i].c];
        uint32_t triangle_color = triangle_list[i].color;

        // get back the vertices of each triangle face
        vector3d v0 = working_vertex_list[triangle_list[i].a];
        vector3d v1 = working_vertex_list[triangle_list[i].b];
        vector3d v2 = working_vertex_list[triangle_list[i].c];

        // Find the two triangle vectors to calculate the face normal
        vector3d vector_ab = { .x = v1.x - v0.x, .y = v1.y - v0.y, .z = v1.z - v0.z };
        vector3d vector_ac = { .x = v2.x - v0.x, .y = v2.y - v0.y, .z = v2.z - v0.z };

        // Normalize the vectors
        normalize(&vector_ab);
        normalize(&vector_ac);

        // Compute the surface normal (through cross-product)
        vector3d normal = {
            .x = (vector_ab.y * vector_ac.z - vector_ab.z * vector_ac.y),
            .y = (vector_ab.z * vector_ac.x - vector_ab.x * vector_ac.z),
            .z = (vector_ab.x * vector_ac.y - vector_ab.y * vector_ac.x)
        };
        normalize(&normal);

        // Get the vector distance between a point in the triangle (v0) and the camera
        vector3d vector_normal_camera = {
            .x = v0.x - camera_position.x,
            .y = v0.y - camera_position.y,
            .z = v0.z - camera_position.z
        };

        // Calculate how similar it is with the normal usinng the dot product
        float dot_normal_camera = dot(normal, vector_normal_camera);

        // only render the triangle that has a positive-z-poiting normal
        if (dot_normal_camera > 0) {
            // Define a vector to represent a light coming from a direction
            vector3d light_direction = { .x = 0, .y = 0, .z = 1 };
            normalize(&light_direction);

            // Shade the triangle based on how aligned is the normal and the light direction
            float light_shade_factor = dot(normal, light_direction);

            // Apply a % light factor to a color
            triangle_color = apply_light(triangle_color, light_shade_factor);

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
            // Draw triangle face lines
            draw_triangle(
                point_a.x + (window_width / 2),
                point_a.y + (window_height / 2),
                point_b.x + (window_width / 2),
                point_b.y + (window_height / 2),
                point_c.x + (window_width / 2),
                point_c.y + (window_height / 2),
                0xFF000000
            );
        }
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
