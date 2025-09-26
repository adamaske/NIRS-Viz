#version 440 core

out vec4 FragColor;

void main()
{
    // Make point a circle
    vec2 coord = gl_PointCoord * 2.0 - 1.0; // [-1,1] range
    if (dot(coord, coord) > 1.0)
        discard;

    FragColor = vec4(1.0, 0.2, 0.2, 1.0); // red sphere
}
