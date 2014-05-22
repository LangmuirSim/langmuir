#version 400

attribute vec4 vertex;
attribute vec4 normal;
uniform mat4 MVP;
uniform mat4 MV;

uniform vec4 light_vertex;
out vec4 n;
out vec4 l;
out vec4 e;

void main(void)
{
    gl_Position = MVP * vertex;

    n = normalize(normal);
    l = normalize(vertex - light_vertex);
    e = normalize(vec4(0,0,0,0) - MV * vertex);


}
