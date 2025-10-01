#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include "Shader.h"

class Line {
public:

	glm::vec3 p1;
	glm::vec3 p2;
	glm::vec3 color;
	float thickness;

	unsigned int vao;
	unsigned int vbo;

	glm::mat4 model = glm::mat4(1.0f);
	Shader* shader;

	Line(const glm::vec3& _p1, const glm::vec3& _p2, const glm::vec3& _color, float _thickness, Shader* _shader);
	~Line();

	bool dirty = false;
	void UpdateMesh();
	void Draw(glm::mat4 view, glm::mat4 proj);
};