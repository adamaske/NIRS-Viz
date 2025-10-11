#include "PointRenderer.h"
#include <glad/glad.h>


PointRenderer::PointRenderer(const float& size, const glm::vec4& _color)
{
	transform = new Transform();
	shader = new Shader(std::string("C:/dev/NIRS-Viz/data/Shaders/point.vert"), std::string("C:/dev/NIRS-Viz/data/Shaders/point.frag"));

	point_size = size;
	color = _color;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Point), points.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);

	glBindVertexArray(0);
}

void PointRenderer::Draw(const glm::mat4 view, const glm::mat4 proj) {
	shader->Bind();
	shader->SetUniform4f("pointColor", color.x, color.y, color.z, color.w);
	shader->SetUniformMat4f("model", transform->GetMatrix());
	shader->SetUniformMat4f("view", view);
	shader->SetUniformMat4f("projection", proj); 
	
	glBindVertexArray(vao);
	glPointSize(point_size);
	glDrawArrays(GL_POINTS, 0, points.size());
	glBindVertexArray(0);
}
void PointRenderer::Update() {
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// No need to bind VAO here, only VBO is needed for data update
	glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Point), points.data(), GL_DYNAMIC_DRAW);
}

void PointRenderer::InsertPoint(const Point& _point) {
	points.push_back(_point);
	Update();
}
void PointRenderer::InsertPoint(const std::vector<Point>& _point)
{
	points.insert(points.end(), _point.begin(), _point.end());
	Update();
}
void PointRenderer::Clear() {
	points.clear();
	Update();
}
