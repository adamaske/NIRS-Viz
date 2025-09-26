#pragma once

#include <glm/glm.hpp>

// A structure to standardize the vertex data used by the Mesh class.
struct Vertex {
    glm::vec3 position; 
    glm::vec3 normal;   
    glm::vec2 tex_coords;


    bool operator==(const Vertex& other) const {
        return position == other.position &&
            normal == other.normal &&
            tex_coords == other.tex_coords;
    };

};