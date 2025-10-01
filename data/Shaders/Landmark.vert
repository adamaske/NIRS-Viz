#version 330 core

layout (location = 0) in vec3 aPos;    // Vertex position
layout (location = 1) in vec3 aNormal; // Normal (not really used here, but good for sphere meshes)

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
