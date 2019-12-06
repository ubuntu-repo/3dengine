#ifndef MESH_DATA_H
#define MESH_DATA_H

const unsigned int N_VERTICES = 8;
const unsigned int N_FACES = 6 * 2; // 6 faces, 2 triangles per face

// Test dynamic array
vec3d vertices[N_VERTICES] = {
    { .x = -1, .y = -1, .z = -1, .w = 1 }, // 0
    { .x = -1, .y =  1, .z = -1, .w = 1 }, // 1
    { .x =  1, .y =  1, .z = -1, .w = 1 }, // 2
    { .x =  1, .y = -1, .z = -1, .w = 1 }, // 3
    { .x =  1, .y =  1, .z =  1, .w = 1 }, // 4
    { .x =  1, .y = -1, .z =  1, .w = 1 }, // 5
    { .x = -1, .y =  1, .z =  1, .w = 1 }, // 6
    { .x = -1, .y = -1, .z =  1, .w = 1 }  // 7
};

arraylist mesh_vertices;

void load_mesh_data() {
    arraylist_init(&mesh_vertices);
    for (int i = 0; i < N_VERTICES; i++) {
        arraylist_add(&mesh_vertices, &vertices[i]);
    }
}

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

#endif
