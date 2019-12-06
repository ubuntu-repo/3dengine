#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "upng.h"
#include "arraylist.h"
#include "graphics.h"
#include "texture.h"
#include "vector.h"
#include "matrix.h"
#include "triangle.h"
#include "mesh_data.h"
#include "texture_data.h"

///////////////////////////////////////////////////////////////////////////////
// Array of updated vertices, triangle faces, and vertex depth values
///////////////////////////////////////////////////////////////////////////////
vec3d projected_points[N_VERTICES];
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
unsigned int previous_frame_time = 0;

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
// Setup function to initialize objects
///////////////////////////////////////////////////////////////////////////////
void setup(void) {
    color_buffer = (uint32_t *) malloc(
        sizeof(uint32_t) * (uint32_t)window_width * (uint32_t) window_height
    );

    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );

    //texture = (uint32_t*) REDBRICK_TEXTURE;
    // allocate the total amount of bytes in memory to hold our wall texture
    mesh_texture = (uint32_t*) malloc(sizeof(uint32_t) * (uint32_t)texture_width * (uint32_t)texture_height);

    // load an external texture using the upng library to decode the file
    png_texture = upng_new_from_file(TEXTURE_FILENAME);
    if (png_texture != NULL) {
        upng_decode(png_texture);
        if (upng_get_error(png_texture) == UPNG_EOK)
            mesh_texture = (uint32_t*)upng_get_buffer(png_texture);
    }

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

    load_mesh_data();
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
        vec3d working_vertex = *(vec3d*)arraylist_get(&mesh_vertices, i);

        // Rotate the original 3d point in the x, y, and z axis
        working_vertex = rotate_x(working_vertex, cube_rotation.x += 0.02 * delta_time);
        working_vertex = rotate_y(working_vertex, cube_rotation.y += 0.03 * delta_time);
        working_vertex = rotate_z(working_vertex, cube_rotation.z += 0.02 * delta_time);

        // After rotation, translate the cube 5 units in the z-axis
        working_vertex.z -= -6.0;

        // Save the rotated and transleted vertex in a list
        working_mesh_vertices[i] = working_vertex;

        // Return the projection of the current point working point
        vec3d projected_point = multiply_vec3d_mat4x4(&working_vertex, &proj_matrix);

        // Scale into view
        projected_point.x *= (float)window_width / 2;
        projected_point.y *= (float)window_height / 2;

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
        vector_normalize(&vector_ab);
        vector_normalize(&vector_ac);

        // Compute the surface normal (through cross-product)
        vec3d normal = {
            .x = (vector_ab.y * vector_ac.z - vector_ab.z * vector_ac.y),
            .y = (vector_ab.z * vector_ac.x - vector_ab.x * vector_ac.z),
            .z = (vector_ab.x * vector_ac.y - vector_ab.y * vector_ac.x)
        };
        vector_normalize(&normal);

        // Get the vector distance between a point in the triangle (v0) and the camera
        vec3d vector_normal_camera = {
            .x = v0.x - camera_position.x,
            .y = v0.y - camera_position.y,
            .z = v0.z - camera_position.z
        };

        // Calculate how similar it is with the normal usinng the dot product
        float dot_normal_camera = vector_dot(normal, vector_normal_camera);

        // only render the triangle that has a positive-z-poiting normal
        if (dot_normal_camera > 0) {
            continue;
        }

        // Define a vector to represent a light coming from a direction
        vec3d light_direction = { .x = 0, .y = 0, .z = -1 };
        vector_normalize(&light_direction);

        // Shade the triangle based on how aligned is the normal and the light direction
        float light_shade_factor = vector_dot(normal, light_direction);

        // Apply a % light factor to a color
        triangle_color = apply_light(triangle_color, light_shade_factor);

        // Draw a textured triangle
        // draw_textured_triangle(
        //     point_a.x, point_a.y, point_a.z, point_a.w, a_uv.u, a_uv.v,
        //     point_b.x, point_b.y, point_b.z, point_b.w, b_uv.u, b_uv.v,
        //     point_c.x, point_c.y, point_c.z, point_c.w, c_uv.u, c_uv.v,
        //     mesh_texture
        // );

        // Draw a filled triangle
        draw_filled_triangle(
            point_a.x, point_a.y,
            point_b.x, point_b.y,
            point_c.x, point_c.y,
            triangle_color
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

    free(color_buffer);
    arraylist_free(&mesh_vertices);

    return 0;
}
