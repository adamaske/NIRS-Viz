#include "Line.h"
#include <spdlog/spdlog.h>
Line::Line(const glm::vec3& _p1, const glm::vec3& _p2, const glm::vec3& _color, float _thickness, Shader* _shader) {
	p1 = _p1;
	p2 = _p2;
	color = _color;
	thickness = _thickness;
    shader = _shader;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
    dirty = true;
}

Line::~Line()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

void Line::UpdateMesh() {
    std::vector<glm::vec3> vertices = { p1, p2 };

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    dirty = false;
}

void Line::Draw(glm::mat4 view, glm::mat4 proj)
{
	if (dirty) UpdateMesh();

    if (!shader) { return;}


    std::vector<glm::vec3> vertices = { p1, p2 };
    
    shader->Bind();

    shader->SetUniformMat4f("model", model);
    shader->SetUniformMat4f("view", view);
    shader->SetUniformMat4f("projection", proj);
    shader->SetUniform3f("lineColor", color);

    glBindVertexArray(vao);
    glLineWidth(thickness);
	glDrawArrays(GL_LINES, 0, vertices.size());
    glBindVertexArray(0);

}
