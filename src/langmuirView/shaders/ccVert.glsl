#version 400

attribute vec4 vertex; // in

void main(void)
{
    gl_Position = vertex;
}
