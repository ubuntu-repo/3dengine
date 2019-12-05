#ifndef TEXTURE_H
#define TEXTURE_H

typedef struct {
    float u;
    float v;
} tex2d;

typedef struct {
    tex2d a_uv;
    tex2d b_uv;
    tex2d c_uv;
} triangle_uv;

///////////////////////////////////////////////////////////////////////////////
// Declare global variables for texture information
///////////////////////////////////////////////////////////////////////////////
uint32_t* mesh_texture = NULL;
upng_t* png_texture = NULL;

int texture_width = 64;
int texture_height = 64;

#endif
