#version 440 core

layout(location = 0) in vec3 aPos; // 3D point position
uniform mat4 model; 
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    gl_PointSize = 8.0; // size in pixels; can also be set dynamically
}
