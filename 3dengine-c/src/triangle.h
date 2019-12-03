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
// Return the barycentric from two points and the point x and y
///////////////////////////////////////////////////////////////////////////////
float barycentric_coord(vec3d p1, vec3d p2, float x, float y) {
    return (p1.y - p2.y) * x + (p2.x - p1.x) * y + p1.x * p2.y - p2.x * p1.y;
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
    int x1, int y1, float z1, float w1, float u1, float v1,
    int x2, int y2, float z2, float w2, float u2, float v2,
    int x3, int y3, float z3, float w3, float u3, float v3,
    uint32_t* texture
) {
    // We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
    if (y2 < y1) {
        swap(&y1, &y2);
        swap(&x1, &x2);
        swapf(&z1, &z2);
        swapf(&w1, &w2);
        swapf(&u1, &u2);
        swapf(&v1, &v2);
    }
    if (y3 < y1) {
        swap(&y1, &y3);
        swap(&x1, &x3);
        swapf(&z1, &z3);
        swapf(&w1, &w3);
        swapf(&u1, &u3);
        swapf(&v1, &v3);
    }
    if (y3 < y2) {
        swap(&y2, &y3);
        swap(&x2, &x3);
        swapf(&z2, &z3);
        swapf(&w2, &w3);
        swapf(&u2, &u3);
        swapf(&v2, &v3);
    }

    // Create Vec3d points from the sorted coordinates
    vec3d point_a = { .x = x1, .y = y1, .z = z1, .w = w1 };
    vec3d point_b = { .x = x2, .y = y2, .z = z2, .w = w2 };
    vec3d point_c = { .x = x3, .y = y3, .z = z3, .w = w3 };

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
                float alpha = barycentric_coord(point_b, point_c, j, i) / barycentric_coord(point_b, point_c, point_a.x, point_a.y);
                float beta = barycentric_coord(point_c, point_a, j, i) / barycentric_coord(point_c, point_a, point_b.x, point_b.y);
                float gamma = barycentric_coord(point_a, point_b, j, i) / barycentric_coord(point_a, point_b, point_c.x, point_c.y);

                float tw = 1, tx = 1, ty = 1;

                if (point_a.z != 0.0 && point_b.z != 0.0 && point_c.z != 0.0) {
                    tw = (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gamma;
                    tx = (u1 / point_a.w) * alpha + (u2 / point_b.w) * beta + (u3 / point_c.w) * gamma;
                    ty = (v1 / point_a.w) * alpha + (v2 / point_b.w) * beta + (v3 / point_c.w) * gamma;
                } else {
                    tx = (u1) * alpha + (u2) * beta + (u3) * gamma;
                    ty = (v1) * alpha + (v2) * beta + (v3) * gamma;
                }

                // Scale texture coordinates to match texture width and height
                tx = (tx * texture_width) / tw;
                ty = (ty * texture_height) / tw;

                tx = abs((int)tx % texture_width);
                ty = abs((int)ty % texture_height);

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
                float alpha = barycentric_coord(point_b, point_c, j, i) / barycentric_coord(point_b, point_c, point_a.x, point_a.y);
                float beta = barycentric_coord(point_c, point_a, j, i) / barycentric_coord(point_c, point_a, point_b.x, point_b.y);
                float gamma = barycentric_coord(point_a, point_b, j, i) / barycentric_coord(point_a, point_b, point_c.x, point_c.y);

                float tw = 1, tx = 1, ty = 1;

                if (point_a.z != 0.0 && point_b.z != 0.0 && point_c.z != 0.0) {
                    tw = (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gamma;
                    tx = (u1 / point_a.w) * alpha + (u2 / point_b.w) * beta + (u3 / point_c.w) * gamma;
                    ty = (v1 / point_a.w) * alpha + (v2 / point_b.w) * beta + (v3 / point_c.w) * gamma;
                } else {
                    tx = (u1) * alpha + (u2) * beta + (u3) * gamma;
                    ty = (v1) * alpha + (v2) * beta + (v3) * gamma;
                }

                // Scale texture coordinates to match texture width and height
                tx = (tx * texture_width) / tw;
                ty = (ty * texture_height) / tw;

                tx = (int)tx % texture_width;
                ty = (int)ty % texture_height;

                // Draw a pixel sampling the texel color from the texture buffer
                draw_pixel(j, i, texture[(texture_width * (int)ty) + (int)tx]);
            }
        }
    }
}

#endif
