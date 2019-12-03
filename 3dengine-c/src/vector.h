#ifndef VECTOR_H
#define VECTOR_H

///////////////////////////////////////////////////////////////////////////////
// Type definition for 2D and 3D Vectors
///////////////////////////////////////////////////////////////////////////////
typedef struct {
    float x;
    float y;
} vec2d;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} vec3d;

///////////////////////////////////////////////////////////////////////////////
// Dot product between two vectors
///////////////////////////////////////////////////////////////////////////////
float dot(vec3d a, vec3d b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
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

#endif
