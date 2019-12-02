#ifndef MATRIX_H
#define MATRIX_H

///////////////////////////////////////////////////////////////////////////////
// Type definition for 4x4 Matrices
///////////////////////////////////////////////////////////////////////////////
typedef struct {
    float m[4][4];
} mat4x4;

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
        /* result_vector.z /= w; */ // some implementations also normalize z
    }
    return result_vector;
}

#endif
