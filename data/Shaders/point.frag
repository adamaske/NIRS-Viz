#version 440 core
uniform vec4 pointColor;
out vec4 FragColor;
void main()
{
    // Make point a circle
    vec2 coord = gl_PointCoord * 2.0 - 1.0; // [-1,1] range
    if (dot(coord, coord) > 1.0)
        discard;

    FragColor = pointColor; 
}
