#version 400

attribute vec4 vertex;
uniform mat4 MVP;
uniform mat4 MV;

void main(void)
{
    gl_Position = MVP * vertex;
}
