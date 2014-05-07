#version 400

attribute vec4 vertex; // in
uniform   mat4 matrix;

void main(void)
{
    gl_Position = matrix * vertex;
}
