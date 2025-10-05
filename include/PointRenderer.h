#pragma

#include <glm/glm.hpp>
#include <vector>

#include "Transform.h"
#include "Shader.h"

using Point = glm::vec3;
class PointRenderer {
public:

	PointRenderer(const float& size, const glm::vec4& _color);

	float point_size = 10.0f;
	glm::vec4 color;
	std::vector<Point> points;
	
	Shader* shader;
	Transform* transform;


	unsigned int vao;
	unsigned int vbo;

	void Draw(const glm::mat4 view, const glm::mat4 proj);
	void Update();
	void InsertPoint(const Point& _point);
	void InsertPoint(const std::vector<Point>& _point);
	void Clear();
};