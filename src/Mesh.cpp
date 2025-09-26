#include "Mesh.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <spdlog/spdlog.h>

Mesh::Mesh(const fs::path& obj_filepath)
{
	// Read OBJ file directly, 
	LoadObj(obj_filepath.string(), vertices, indices);
    Init();

    // Debug OBJ
	spdlog::info("Loaded OBJ file: {}", obj_filepath.string());
	spdlog::info("Vertices count: {}", vertices.size());
	spdlog::info("Indices count: {}", indices.size());
}

Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void Mesh::Init() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

   glBindVertexArray(VAO);

   glBindBuffer(GL_ARRAY_BUFFER, VBO);
   glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);


   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

   glEnableVertexAttribArray(1);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

   glEnableVertexAttribArray(2);
   glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));

   glBindVertexArray(0);

}


namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& v) const noexcept {
            size_t h1 = hash<float>()(v.position.x) ^ (hash<float>()(v.position.y) << 1) ^ (hash<float>()(v.position.z) << 2);
            size_t h2 = hash<float>()(v.normal.x) ^ (hash<float>()(v.normal.y) << 1) ^ (hash<float>()(v.normal.z) << 2);
            size_t h3 = hash<float>()(v.tex_coords.x) ^ (hash<float>()(v.tex_coords.y) << 1);
            return h1 ^ h2 ^ h3;
        }
    };
}

bool Mesh::LoadObj(const std::string& filename,
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open OBJ file: " << filename << std::endl;
        return false;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;

    std::unordered_map<Vertex, unsigned int> uniqueVertices;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "v") {
            glm::vec3 pos;
            ss >> pos.x >> pos.y >> pos.z;
            positions.push_back(pos);
        }
        else if (type == "vt") {
            glm::vec2 uv;
            ss >> uv.x >> uv.y;
            texCoords.push_back(uv);
        }
        else if (type == "vn") {
            glm::vec3 n;
            ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (type == "f") {
            std::string vertexStr;
            for (int i = 0; i < 3; i++) { // assumes triangulated OBJ
                ss >> vertexStr;
                std::istringstream vs(vertexStr);
                std::string vIdx, tIdx, nIdx;

                std::getline(vs, vIdx, '/');
                std::getline(vs, tIdx, '/');
                std::getline(vs, nIdx, '/');

                int vi = std::stoi(vIdx) - 1;
                int ti = tIdx.empty() ? -1 : std::stoi(tIdx) - 1;
                int ni = nIdx.empty() ? -1 : std::stoi(nIdx) - 1;

                Vertex v{};
                v.position = positions[vi];
                if (ti >= 0) v.tex_coords = texCoords[ti];
                if (ni >= 0) v.normal = normals[ni];

                if (uniqueVertices.count(v) == 0) {
                    uniqueVertices[v] = static_cast<unsigned int>(vertices.size());
                    vertices.push_back(v);
                }

                indices.push_back(uniqueVertices[v]);
            }
        }
    }

    return true;
}