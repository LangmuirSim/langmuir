#version 400

uniform float pointSize;
uniform mat4 matrix;
in vec4 vertex;

void main(void)
{
    gl_Position = matrix * vertex;
    gl_PointSize = pointSize;
}
