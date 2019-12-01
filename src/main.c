#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "defs.h"
#include "textures.h"

///////////////////////////////////////////////////////////////////////////////
// Declare cube vertices and triangles
///////////////////////////////////////////////////////////////////////////////
const unsigned int N_VERTICES = 8;
vec3d mesh_vertices[N_VERTICES] = {
    { .x = -1, .y = -1, .z = -1 }, // 0
    { .x = -1, .y =  1, .z = -1 }, // 1
    { .x =  1, .y =  1, .z = -1 }, // 2
    { .x =  1, .y = -1, .z = -1 }, // 3
    { .x =  1, .y =  1, .z =  1 }, // 4
    { .x =  1, .y = -1, .z =  1 }, // 5
    { .x = -1, .y =  1, .z =  1 }, // 6
    { .x = -1, .y = -1, .z =  1 }  // 7
};

const unsigned int N_FACES = 6 * 2; // 6 faces, 2 triangles per face
triangle mesh_faces[N_FACES] = {
    // front
    { .a = 1, .b = 2, .c = 3, .color = 0xFFFF0000, .face_index = 0 },
    { .a = 1, .b = 3, .c = 4, .color = 0xFFFF0000, .face_index = 1 },
    // right
    { .a = 4, .b = 3, .c = 5, .color = 0xFF00FF00, .face_index = 2 },
    { .a = 4, .b = 5, .c = 6, .color = 0xFF00FF00, .face_index = 3 },
    // // back
    { .a = 6, .b = 5, .c = 7, .color = 0xFF0000FF, .face_index = 4 },
    { .a = 6, .b = 7, .c = 8, .color = 0xFF0000FF, .face_index = 5 },
    // left
    { .a = 8, .b = 7, .c = 2, .color = 0xFFFFFF00, .face_index = 6 },
    { .a = 8, .b = 2, .c = 1, .color = 0xFFFFFF00, .face_index = 7 },
    // top
    { .a = 2, .b = 7, .c = 5, .color = 0xFF00FFFF, .face_index = 8 },
    { .a = 2, .b = 5, .c = 3, .color = 0xFF00FFFF, .face_index = 9 },
    // bottom
    { .a = 6, .b = 8, .c = 1, .color = 0xFFFFFFFF, .face_index = 10 },
    { .a = 6, .b = 1, .c = 4, .color = 0xFFFFFFFF, .face_index = 11 }
};

triangle_uv mesh_faces_uvs[N_FACES] = {
    // front
    { .a_uv = { 0, 1 }, .b_uv = { 0, 0 }, .c_uv = { 1, 0 } },
    { .a_uv = { 0, 1 }, .b_uv = { 1, 0 }, .c_uv = { 1, 1 } },
    // right
    { .a_uv = { 0, 1 }, .b_uv = { 0, 0 }, .c_uv = { 1, 0 } },
    { .a_uv = { 0, 1 }, .b_uv = { 1, 0 }, .c_uv = { 1, 1 } },
    // back
    { .a_uv = { 0, 1 }, .b_uv = { 0, 0 }, .c_uv = { 1, 0 } },
    { .a_uv = { 0, 1 }, .b_uv = { 1, 0 }, .c_uv = { 1, 1 } },
    // left
    { .a_uv = { 0, 1 }, .b_uv = { 0, 0 }, .c_uv = { 1, 0 } },
    { .a_uv = { 0, 1 }, .b_uv = { 1, 0 }, .c_uv = { 1, 1 } },
    // top
    { .a_uv = { 0, 1 }, .b_uv = { 0, 0 }, .c_uv = { 1, 0 } },
    { .a_uv = { 0, 1 }, .b_uv = { 1, 0 }, .c_uv = { 1, 1 } },
    // bottom
    { .a_uv = { 0, 1 }, .b_uv = { 0, 0 }, .c_uv = { 1, 0 } },
    { .a_uv = { 0, 1 }, .b_uv = { 1, 0 }, .c_uv = { 1, 1 } }
};

///////////////////////////////////////////////////////////////////////////////
// Array of updated vertices, triangle faces, and vertex depth values
///////////////////////////////////////////////////////////////////////////////
vec3d projected_points[N_VERTICES];
triangle_uv projected_uvs[N_VERTICES];
float vertex_depth_list[N_VERTICES];
vec3d working_mesh_vertices[N_VERTICES];

///////////////////////////////////////////////////////////////////////////////
// Projection matrix
///////////////////////////////////////////////////////////////////////////////
mat4x4 proj_matrix;

///////////////////////////////////////////////////////////////////////////////
// Declare the camera position, rotation, and FOV distortion variables
///////////////////////////////////////////////////////////////////////////////
float fov_factor = 640.0f;
vec3d camera_position = { .x = 0, .y = 0, .z = 0 };
vec3d cube_rotation = { .x = 0, .y = 0, .z = 0 };

///////////////////////////////////////////////////////////////////////////////
// Global variables for SDL Window, Renderer, and execution status
///////////////////////////////////////////////////////////////////////////////
bool is_running = false;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
unsigned window_width = 800;
unsigned window_height = 600;
unsigned int previous_frame_time = 0;

///////////////////////////////////////////////////////////////////////////////
// Declare a color buffer array and a texture that will be used to display it
///////////////////////////////////////////////////////////////////////////////
uint32_t* color_buffer = NULL;
SDL_Texture* color_buffer_texture;

///////////////////////////////////////////////////////////////////////////////
// Declare texture variabless
///////////////////////////////////////////////////////////////////////////////
uint32_t* texture = NULL;
int texture_width = 64;
int texture_height = 64;

///////////////////////////////////////////////////////////////////////////////
// Functions to rotate 3D points in X, Y, and Z
///////////////////////////////////////////////////////////////////////////////
vec3d rotate_x(vec3d point, float angle) {
    vec3d working_vertex = {
        .x = point.x,
        .y = cos(angle) * point.y - sin(angle) * point.z,
        .z = sin(angle) * point.y + cos(angle) * point.z
    };
    return working_vertex;
}

vec3d rotate_y(vec3d point, float angle) {
    vec3d working_vertex = {
        .x = cos(angle) * point.x - sin(angle) * point.z,
        .y = point.y,
        .z = sin(angle) * point.x + cos(angle) * point.z
    };
    return working_vertex;
}

vec3d rotate_z(vec3d point, float angle) {
    vec3d working_vertex = {
        .x = cos(angle) * point.x - sin(angle) * point.y,
        .y = sin(angle) * point.x + cos(angle) * point.y,
        .z = point.z
    };
    return working_vertex;
}

///////////////////////////////////////////////////////////////////////////////
// Function to initialize the SDL Window and Renderer
///////////////////////////////////////////////////////////////////////////////
int initialize_window(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
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
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error creating SDL Renderer.\n");
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Poll system events and handle keyboard presses
///////////////////////////////////////////////////////////////////////////////
void process_input(void) {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
                is_running = false;
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
    for (int y = 0; y < window_height; y++)
        for (int x = 0; x < window_width; x++)
            if (x % 5 == 0 && y % 5 == 0)
                color_buffer[(window_width * y) + x] = 0xFF333333;
            else
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

    // DDA is naive and slow as it uses floating point for step increment
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

///////////////////////////////////////////////////////////////////////////////
// Function to swap the value of two integer variables
///////////////////////////////////////////////////////////////////////////////
void swap(int* a, int* b) {
    int c = *a;
    *a = *b;
    *b = c;
}

void swapf(float* a, float* b) {
    float c = *a;
    *a = *b;
    *b = c;
}

///////////////////////////////////////////////////////////////////////////////
// Function to sort tringle vertices by ascending y-component
///////////////////////////////////////////////////////////////////////////////
void sort_triangle_vertices_y(int* x0, int* y0, int* x1, int* y1, int* x2, int* y2) {
    if (*y0 > *y1) {
        swap(y0, y1);
        swap(x0, x1);
    }
    if (*y1 > *y2) {
        swap(y1, y2);
        swap(x1, x2);
    }
    if (*y0 > *y1) {
        swap(y0, y1);
        swap(x0, x1);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Draw a filled a triangle with a flat bottom
///////////////////////////////////////////////////////////////////////////////
void fill_flat_bottom_triangle(float x0, float y0, float x1, float y1, float x2, float y2, uint32_t color) {
    float inv_slope_left = (x1 - x0) / (y1 - y0);
    float inv_slope_right = (x2 - x0) / (y2 - y0);

    float x_start = x0;
    float x_end = x0;

    for (int y = y0; y <= y1; y++) {
        if (x_start <= x_end) {
            for (int x = (int)x_start; x <= (int)x_end; x++)
                draw_pixel(x, y, color);
        } else {
            for (int x = (int)x_end; x <= (int)x_start; x++)
                draw_pixel(x, y, color);
        }
        x_start += inv_slope_left;
        x_end += inv_slope_right;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Draw a filled a triangle with a flat top
///////////////////////////////////////////////////////////////////////////////
void fill_flat_top_triangle(float x0, float y0, float x1, float y1, float x2, float y2, uint32_t color) {
    float inv_slope_left = (x2 - x0) / (y2 - y0);
    float inv_slope_right = (x2 - x1) / (y2 - y1);

    float x_start = x0;
    float x_end = x1;

    for (int y = y0; y < y2; y++) {
        if (x_start <= x_end) {
            for (int x = (int)x_start; x <= (int)x_end; x++)
                draw_pixel(x, y, color);
        } else {
            for (int x = (int)x_end; x <= (int)x_start; x++)
                draw_pixel(x, y, color);
        }
        x_start += inv_slope_left;
        x_end += inv_slope_right;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Draw a filled triangle with the flat-top/flat-bottom method
// We split the original triangle in two, half flat-bottom and half flat-top
///////////////////////////////////////////////////////////////////////////////
//
//        v0
//        /\
//       /  \
//      /    \
//     /      \
//   v1 - - - -v3
//     \_       \
//        \_     \
//           \_   \
//              \_ \
//                 \\
//                   \
//                    v2
//
///////////////////////////////////////////////////////////////////////////////
void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    // We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
    sort_triangle_vertices_y(&x0, &y0, &x1, &y1, &x2, &y2);

    if (y1 == y2) {
        fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);
    } else if (y0 == y1) {
        fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);
    } else {
        // Create a new vertex (x3,y3) using triangle similarity
        float x3 = (int)(x0 + ((float)(y1 - y0) / (float)(y2 - y0)) * (x2 - x0));
        float y3 = y1;

        fill_flat_bottom_triangle(x0, y0, x1, y1, x3, y3, color);
        fill_flat_top_triangle(x1, y1, x3, y3, x2, y2, color);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Return the barycentric from two points and the point x and y
///////////////////////////////////////////////////////////////////////////////
float barycentric(vec3d p1, vec3d p2, float x, float y) {
    return (p1.y - p2.y) * x + (p2.x - p1.x) * y + p1.x * p2.y - p2.x * p1.y;
}

///////////////////////////////////////////////////////////////////////////////
// Draw a textured triangle with the flat-top/flat-bottom method
// We split the original triangle in two, half flat-bottom and half flat-top
///////////////////////////////////////////////////////////////////////////////
//
//        v0
//        /\
//       /  \
//      /    \
//     /      \
//   v1 - - - -v3
//     \_       \
//        \_     \
//           \_   \
//              \_ \
//                 \\
//                   \
//                    v2
//
///////////////////////////////////////////////////////////////////////////////
void draw_textured_triangle(
    int x1, int y1, float z1, float u1, float v1,
    int x2, int y2, float z2, float u2, float v2,
    int x3, int y3, float z3, float u3, float v3,
    uint32_t* texture
) {
    // We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
    if (y2 < y1) {
        swap(&y1, &y2);
        swap(&x1, &x2);
        swapf(&z1, &z2);
        swapf(&u1, &u2);
        swapf(&v1, &v2);
    }
    if (y3 < y1) {
        swap(&y1, &y3);
        swap(&x1, &x3);
        swapf(&z1, &z3);
        swapf(&u1, &u3);
        swapf(&v1, &v3);
    }
    if (y3 < y2) {
        swap(&y2, &y3);
        swap(&x2, &x3);
        swapf(&z2, &z3);
        swapf(&u2, &u3);
        swapf(&v2, &v3);
    }

    // Create Vec3d points from the sorted coordinates
    vec3d point_a = { .x = x1, .y = y1, .z = z1 };
    vec3d point_b = { .x = x2, .y = y2, .z = z2 };
    vec3d point_c = { .x = x3, .y = y3, .z = z3 };

    /////////////////////////////////////////////////////////////
    // Render first triangle (flat-bottom)
    /////////////////////////////////////////////////////////////
    int dy1 = y2 - y1;
    int dx1 = x2 - x1;

    int dy2 = y3 - y1;
    int dx2 = x3 - x1;

    float dax_step = 0, dbx_step = 0;

    if (dy1) dax_step = dx1 / (float)abs(dy1);
    if (dy2) dbx_step = dx2 / (float)abs(dy2);

    if (dy1) {
        for (int i = y1; i <= y2; i++) {
            int ax = x1 + (float)(i - y1) * dax_step;
            int bx = x1 + (float)(i - y1) * dbx_step;

            if (ax > bx) {
                swap(&ax, &bx);
            }

            for (int j = ax; j < bx; j++) {
                float alpha = barycentric(point_b, point_c, j, i) / barycentric(point_b, point_c, point_a.x, point_a.y);
                float beta = barycentric(point_c, point_a, j, i) / barycentric(point_c, point_a, point_b.x, point_b.y);
                float gamma = barycentric(point_a, point_b, j, i) / barycentric(point_a, point_b, point_c.x, point_c.y);

                float w = (1 / point_a.z) * alpha + (1 / point_b.z) * beta + (1 / point_c.z) * gamma;

                float tx = (u1 / point_a.z) * alpha + (u2 / point_b.z) * beta + (u3 / point_c.z) * gamma;
                float ty = (v1 / point_a.z) * alpha + (v2 / point_b.z) * beta + (v3 / point_c.z) * gamma;

                // Scale texture coordinates to match texture width and height
                tx = (tx * texture_width) / w;
                ty = (ty * texture_height) / w;

                tx = (int)tx % texture_width;
                ty = (int)ty % texture_height;

                // Draw a pixel sampling the texel color from the texture buffer
                draw_pixel(j, i, texture[(texture_width * (int)ty) + (int)tx]);
            }
        }
    }

    /////////////////////////////////////////////////////////////
    // Render second triangle (flat-top)
    /////////////////////////////////////////////////////////////
    dy1 = y3 - y2;
    dx1 = x3 - x2;

    if (dy1) dax_step = dx1 / (float)abs(dy1);
    if (dy2) dbx_step = dx2 / (float)abs(dy2);

    if (dy1) {
        for (int i = y2; i <= y3; i++) {
            int ax = x2 + (float)(i - y2) * dax_step;
            int bx = x1 + (float)(i - y1) * dbx_step;

            if (ax > bx) {
                swap(&ax, &bx);
            }

            for (int j = ax; j < bx; j++) {
                float alpha = barycentric(point_b, point_c, j, i) / barycentric(point_b, point_c, point_a.x, point_a.y);
                float beta = barycentric(point_c, point_a, j, i) / barycentric(point_c, point_a, point_b.x, point_b.y);
                float gamma = barycentric(point_a, point_b, j, i) / barycentric(point_a, point_b, point_c.x, point_c.y);

                float w = (1 / point_a.z) * alpha + (1 / point_b.z) * beta + (1 / point_c.z) * gamma;

                float tx = (u1 / point_a.z) * alpha + (u2 / point_b.z) * beta + (u3 / point_c.z) * gamma;
                float ty = (v1 / point_a.z) * alpha + (v2 / point_b.z) * beta + (v3 / point_c.z) * gamma;

                // Scale texture coordinates to match texture width and height
                tx = (tx * texture_width) / w;
                ty = (ty * texture_height) / w;

                tx = (int)tx % texture_width;
                ty = (int)ty % texture_height;

                // Draw a pixel sampling the texel color from the texture buffer
                draw_pixel(j, i, texture[(texture_width * (int)ty) + (int)tx]);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Normalize a vector
///////////////////////////////////////////////////////////////////////////////
void normalize(vec3d* vector) {
    float length = sqrt(
        (vector->x * vector->x) +
        (vector->y * vector->y) +
        (vector->z * vector->z)
    );
    vector->x /= length;
    vector->y /= length;
    vector->z /= length;
}

///////////////////////////////////////////////////////////////////////////////
// Function to multiply a 3D Vector by a 4x4 Matrix
///////////////////////////////////////////////////////////////////////////////
vec3d multiply_vec3d_mat4x4(vec3d* v, mat4x4* m) {
    vec3d result_vector = {
        .x = v->x * m->m[0][0] + v->y * m->m[1][0] + v->z * m->m[2][0] + m->m[3][0],
        .y = v->x * m->m[0][1] + v->y * m->m[1][1] + v->z * m->m[2][1] + m->m[3][1],
        .z = v->x * m->m[0][2] + v->y * m->m[1][2] + v->z * m->m[2][2] + m->m[3][2]
    };
    float w = v->x * m->m[0][3] + v->y * m->m[1][3] + v->z * m->m[2][3] + m->m[3][3];
    if (w != 0.0) {
        result_vector.x /= w;
        result_vector.y /= w;
        result_vector.z /= w;
    }
    return result_vector;
}

///////////////////////////////////////////////////////////////////////////////
// Dot product between two vectors
///////////////////////////////////////////////////////////////////////////////
uint32_t apply_light(uint32_t color, float percentage_factor) {
    uint32_t a = (color & 0xFF000000);
    uint32_t r = (color & 0x00FF0000) * percentage_factor;
    uint32_t g = (color & 0x0000FF00) * percentage_factor;
    uint32_t b = (color & 0x000000FF) * percentage_factor;
    uint32_t new_color = a | (r & 0x00FF0000) | (g & 0x0000FF00) | (b & 0x000000FF);
    return new_color;
}

///////////////////////////////////////////////////////////////////////////////
// Dot product between two vectors
///////////////////////////////////////////////////////////////////////////////
float dot(vec3d a, vec3d b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

///////////////////////////////////////////////////////////////////////////////
// Setup function to initialize objects
///////////////////////////////////////////////////////////////////////////////
void setup(void) {
    color_buffer = (uint32_t *) malloc(
        sizeof(uint32_t) * (uint32_t)window_width * (uint32_t) window_height
    );

    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );

    texture = (uint32_t*) REDBRICK_TEXTURE;

    // Initialize the projection matrix elements
    float aspect_ratio = ((float)window_height / (float)window_width);
    float fov = 60.0; // degrees
    float fov_scale = 1 / tan((fov / 2) / 180.0 * M_PI); // radians
    float znear = 0.1;
    float zfar = 100.0;

    proj_matrix.m[0][0] = aspect_ratio * fov_scale;
    proj_matrix.m[1][1] = fov_scale;
    proj_matrix.m[2][2] = zfar / (zfar - znear);
    proj_matrix.m[3][2] = (-zfar * znear) / (zfar - znear);
    proj_matrix.m[2][3] = 1.0;
}

///////////////////////////////////////////////////////////////////////////////
// Update function with a fixed time step
///////////////////////////////////////////////////////////////////////////////
void update(void) {
    // Waste some time / sleep until we reach the frame target time
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), previous_frame_time + FRAME_TARGET_TIME));

    // Get a delta time factor converted to seconds to be used to update my objects
    float delta_time = (SDL_GetTicks() - previous_frame_time) / 1000.0f;

    // Store the milliseconds of the current frame
    previous_frame_time = SDL_GetTicks();

    // Loop all cube vertices, rotating and projecting them
    for (int i = 0; i < N_VERTICES; i++) {
        vec3d current_point = mesh_vertices[i];

        // Rotate the original 3d point in the x, y, and z axis
        vec3d working_vertex = current_point;
        working_vertex = rotate_x(working_vertex, cube_rotation.x += 0.04 * delta_time);
        working_vertex = rotate_y(working_vertex, cube_rotation.y += 0.03 * delta_time);
        working_vertex = rotate_z(working_vertex, cube_rotation.z += 0.02 * delta_time);

        // After rotation, translate the cube 5 units in the z-axis
        working_vertex.z -= -6.0;

        // Save the rotated and transleted vertex in a list
        working_mesh_vertices[i] = working_vertex;

        // return the projection of the current point working point
        // vec3d projected_point = multiply_vec3d_mat4x4(&working_vertex, &proj_matrix);

        vec3d projected_point = {
            .x = (640.0 * working_vertex.x) / working_vertex.z,
            .y = (640.0 * working_vertex.y) / working_vertex.z,
            .z = working_vertex.z
        };

        // Scale into view
        //projected_point.x *= (float)window_width / 2;
        //projected_point.y *= (float)window_height / 2;

        // Translate into view
        projected_point.x += (float)window_width / 2;
        projected_point.y += (float)window_height / 2;

        // Save the 2d projected points
        projected_points[i] = projected_point;

        // Save the depth of all vertices
        vertex_depth_list[i] = working_vertex.z;
    }

    // calculate the average z-depth of each triangle
    float average_depth_list[N_FACES];
    for (int i = 0; i < N_FACES; i++) {
        average_depth_list[i] = vertex_depth_list[mesh_faces[i].a - 1];
        average_depth_list[i] += vertex_depth_list[mesh_faces[i].b - 1];
        average_depth_list[i] += vertex_depth_list[mesh_faces[i].c - 1];
        average_depth_list[i] /= 3.0;
    }

    // sort triangles by their average depth value
    for (int i = 0; i < N_FACES; i++) {
        for (int j = 0; j < N_FACES - 1; j++) {
            if (average_depth_list[i] > average_depth_list[j]) {
                // swap the triangls in the original triangle list
                triangle temp_triangle = mesh_faces[i];
                mesh_faces[i] = mesh_faces[j];
                mesh_faces[j] = temp_triangle;
                // also swap the depth value in the depth array
                float temp_depth = average_depth_list[i];
                average_depth_list[i] = average_depth_list[j];
                average_depth_list[j] = temp_depth;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Render function to draw objects on the display
///////////////////////////////////////////////////////////////////////////////
void render(void) {
    // Clear the render background with a black color
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Loop all cube face triangles to render them one by one
    for (int i = 0; i < N_FACES; i++) {
        vec3d point_a = projected_points[mesh_faces[i].a - 1];
        vec3d point_b = projected_points[mesh_faces[i].b - 1];
        vec3d point_c = projected_points[mesh_faces[i].c - 1];

        uint32_t triangle_color = mesh_faces[i].color;

        // Get back the vertices of each triangle face
        vec3d v0 = working_mesh_vertices[mesh_faces[i].a - 1];
        vec3d v1 = working_mesh_vertices[mesh_faces[i].b - 1];
        vec3d v2 = working_mesh_vertices[mesh_faces[i].c - 1];

        // Get the triangle UV coordinates
        tex2d a_uv = mesh_faces_uvs[mesh_faces[i].face_index].a_uv;
        tex2d b_uv = mesh_faces_uvs[mesh_faces[i].face_index].b_uv;
        tex2d c_uv = mesh_faces_uvs[mesh_faces[i].face_index].c_uv;

        // Find the two triangle vectors to calculate the face normal
        vec3d vector_ab = { .x = v1.x - v0.x, .y = v1.y - v0.y, .z = v1.z - v0.z };
        vec3d vector_ac = { .x = v2.x - v0.x, .y = v2.y - v0.y, .z = v2.z - v0.z };

        // Normalize the vectors
        normalize(&vector_ab);
        normalize(&vector_ac);

        // Compute the surface normal (through cross-product)
        vec3d normal = {
            .x = (vector_ab.y * vector_ac.z - vector_ab.z * vector_ac.y),
            .y = (vector_ab.z * vector_ac.x - vector_ab.x * vector_ac.z),
            .z = (vector_ab.x * vector_ac.y - vector_ab.y * vector_ac.x)
        };
        normalize(&normal);

        // Get the vector distance between a point in the triangle (v0) and the camera
        vec3d vector_normal_camera = {
            .x = v0.x - camera_position.x,
            .y = v0.y - camera_position.y,
            .z = v0.z - camera_position.z
        };

        // Calculate how similar it is with the normal usinng the dot product
        float dot_normal_camera = dot(normal, vector_normal_camera);

        // only render the triangle that has a positive-z-poiting normal
        if (dot_normal_camera > 0) {
            continue;
        }

        // Define a vector to represent a light coming from a direction
        vec3d light_direction = { .x = 0, .y = 0, .z = -1 };
        normalize(&light_direction);

        // Shade the triangle based on how aligned is the normal and the light direction
        float light_shade_factor = dot(normal, light_direction);

        // Apply a % light factor to a color
        triangle_color = apply_light(triangle_color, light_shade_factor);

        // Draw a filled triangle
        // draw_filled_triangle(
        //     point_a.x, point_a.y,
        //     point_b.x, point_b.y,
        //     point_c.x, point_c.y,
        //     triangle_color
        // );

        // Draw a textured triangle
        draw_textured_triangle(
            point_a.x, point_a.y, point_a.z, a_uv.u, a_uv.v,
            point_b.x, point_b.y, point_b.z, b_uv.u, b_uv.v,
            point_c.x, point_c.y, point_c.z, c_uv.u, c_uv.v,
            texture
        );

        // Draw triangle face lines
        // draw_triangle(
        //     point_a.x, point_a.y,
        //     point_b.x, point_b.y,
        //     point_c.x, point_c.y,
        //     0xFF000000
        // );
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
    is_running = initialize_window();

    setup();

    while (is_running) {
        process_input();
        update();
        render();
    }

    destroy_window();

    return 0;
}
