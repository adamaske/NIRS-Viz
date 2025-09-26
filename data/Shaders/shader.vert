#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;      // NEW: This transforms object space to world space
uniform mat4 view;      
uniform mat4 projection;

void main()
{
    // The final vertex position in clip space is calculated as:
    // P * V * M * Local_Position
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}