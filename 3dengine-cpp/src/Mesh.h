#ifndef MESH_H
#define MESH_H

#include <vector>
#include "Triangle.h"

class Mesh {
    public:
        std::vector<Triangle*> triangles;
};

#endif
