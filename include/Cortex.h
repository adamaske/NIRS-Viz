#pragma once

#include "Transform.h"
#include "Mesh.h"
#include "Shader.h"

class Cortex {
public:

	Cortex();

	Shader shader;
	Mesh mesh;
	Transform transform;

	void Draw(glm::mat4 view, glm::mat4 proj, glm::vec3 view_position);
};