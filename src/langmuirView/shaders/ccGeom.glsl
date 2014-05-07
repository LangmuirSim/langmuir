#version 400

layout(points) in;
layout(points) out;
layout(max_vertices = 1) out;

void main(void)
{
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    EndPrimitive();
}
