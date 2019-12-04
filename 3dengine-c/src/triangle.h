#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "graphics.h"
#include "texture.h"

///////////////////////////////////////////////////////////////////////////////
// Type definition for Triangles
///////////////////////////////////////////////////////////////////////////////
typedef struct {
    int a;
    int b;
    int c;
    uint32_t color;
    int face_index;
} triangle;

///////////////////////////////////////////////////////////////////////////////
// Function to swap the value of two integer variables
///////////////////////////////////////////////////////////////////////////////
void swapi(int* a, int* b) {
    int c = *a;
    *a = *b;
    *b = c;
}

///////////////////////////////////////////////////////////////////////////////
// Function to swap the value of two float variables
///////////////////////////////////////////////////////////////////////////////
void swapf(float* a, float* b) {
    float c = *a;
    *a = *b;
    *b = c;
}

///////////////////////////////////////////////////////////////////////////////
// Return the triangle area using the cross product
///////////////////////////////////////////////////////////////////////////////
float area_triangle(vec3d p1, vec3d p2, vec3d p3) {
    float cross_magnitude = vector_length(vector_cross(vector_sub(p1, p2), vector_sub(p3, p2)));
    return cross_magnitude / 2;
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
    if (y0 > y1) {
        swapi(&y0, &y1);
        swapi(&x0, &x1);
    }
    if (y1 > y2) {
        swapi(&y1, &y2);
        swapi(&x1, &x2);
    }
    if (y0 > y1) {
        swapi(&y0, &y1);
        swapi(&x0, &x1);
    }

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


vec2d get_texel_coords(
    vec3d point_a, vec3d point_b, vec3d point_c, vec3d point_p,
    float u0, float v0, float u1, float v1, float u2, float v2
) {
    float alpha = area_triangle(point_c, point_b, point_p) / area_triangle(point_a, point_b, point_c);
    float beta = area_triangle(point_a, point_c, point_p) / area_triangle(point_a, point_b, point_c);
    float gamma = area_triangle(point_a, point_b, point_p) / area_triangle(point_a, point_b, point_c);

    // printf("ALPHA=%.2f, BETA=%.2f, GAMMA=%.2f, (A+B+G)=%.2f\n", alpha, beta, gamma, alpha+beta+gamma);

    float tw = 1, tx = 1, ty = 1;

    if (point_a.z != 0.0 && point_b.z != 0.0 && point_c.z != 0.0) {
        tw = (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gamma;
        tx = (u0 / point_a.w) * alpha + (u1 / point_b.w) * beta + (u2 / point_c.w) * gamma;
        ty = (v0 / point_a.w) * alpha + (v1 / point_b.w) * beta + (v2 / point_c.w) * gamma;
    }

    // Scale texture coordinates to match texture width and height
    tx = (tx * texture_width) / tw;
    ty = (ty * texture_height) / tw;

    tx = abs((int)tx % texture_width);
    ty = abs((int)ty % texture_height);

    vec2d texel_coords = {
        .x = abs((int)tx % texture_width),
        .y = abs((int)ty % texture_height)
    };
    return texel_coords;
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
    int x0, int y0, float z0, float w0, float u0, float v0,
    int x1, int y1, float z1, float w1, float u1, float v1,
    int x2, int y2, float z2, float w2, float u2, float v2,
    uint32_t* texture
) {
    // We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
    if (y1 < y0) {
        swapi(&y0, &y1);
        swapi(&x0, &x1);
        swapf(&z0, &z1);
        swapf(&w0, &w1);
        swapf(&u0, &u1);
        swapf(&v0, &v1);
    }
    if (y2 < y0) {
        swapi(&y0, &y2);
        swapi(&x0, &x2);
        swapf(&z0, &z2);
        swapf(&w0, &w2);
        swapf(&u0, &u2);
        swapf(&v0, &v2);
    }
    if (y2 < y1) {
        swapi(&y1, &y2);
        swapi(&x1, &x2);
        swapf(&z1, &z2);
        swapf(&w1, &w2);
        swapf(&u1, &u2);
        swapf(&v1, &v2);
    }

    // Create vec3d points from the sorted coordinates
    vec3d point_a = { .x = x0, .y = y0, .z = z0, .w = w0 };
    vec3d point_b = { .x = x1, .y = y1, .z = z1, .w = w1 };
    vec3d point_c = { .x = x2, .y = y2, .z = z2, .w = w2 };

    /////////////////////////////////////////////////////////////
    // Render first triangle (flat-bottom)
    /////////////////////////////////////////////////////////////
    int dy1 = y1 - y0;
    int dx1 = x1 - x0;

    int dy2 = y2 - y0;
    int dx2 = x2 - x0;

    float dax_step = 0, dbx_step = 0;

    if (dy1) dax_step = dx1 / (float)abs(dy1);
    if (dy2) dbx_step = dx2 / (float)abs(dy2);

    if (dy1) {
        for (int y = y0; y <= y1; y++) {
            int ax = x0 + (float)(y - y0) * dax_step;
            int bx = x0 + (float)(y - y0) * dbx_step;

            if (ax > bx) {
                swapi(&ax, &bx);
            }

            for (int x = ax; x < bx; x++) {
                vec3d point_p = { .x = x, .y = y };
                vec2d tex_coords = get_texel_coords(point_a, point_b, point_c, point_p, u0, v0, u1, v1, u2, v2);

                // Draw a pixel sampling the texel color from the texture buffer
                draw_pixel(x, y, texture[(texture_width * (int)tex_coords.y) + (int)tex_coords.x]);
            }
        }
    }

    /////////////////////////////////////////////////////////////
    // Render second triangle (flat-top)
    /////////////////////////////////////////////////////////////
    dy1 = y2 - y1;
    dx1 = x2 - x1;

    if (dy1) dax_step = dx1 / (float)abs(dy1);
    if (dy2) dbx_step = dx2 / (float)abs(dy2);

    if (dy1) {
        for (int y = y1; y <= y2; y++) {
            int ax = x1 + (float)(y - y1) * dax_step;
            int bx = x0 + (float)(y - y0) * dbx_step;

            if (ax > bx) {
                swapi(&ax, &bx);
            }

            for (int x = ax; x < bx; x++) {
                vec3d point_p = { .x = x, .y = y };
                vec2d tex_coords = get_texel_coords(point_a, point_b, point_c, point_p, u0, v0, u1, v1, u2, v2);

                // Draw a pixel sampling the texel color from the texture buffer
                draw_pixel(x, y, texture[(texture_width * (int)tex_coords.y) + (int)tex_coords.x]);
            }
        }
    }
}

#endif
