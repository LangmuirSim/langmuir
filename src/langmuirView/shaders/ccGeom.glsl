#version 400

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

uniform mat4 matrix;

void main(void)
{
    gl_Position = matrix * (gl_in[0].gl_Position + vec4(-0.4,-0.4, 0.0, 0.0));
    EmitVertex();

    gl_Position = matrix * (gl_in[0].gl_Position + vec4(-0.4,+0.4, 0.0, 0.0));
    EmitVertex();

    gl_Position = matrix * (gl_in[0].gl_Position + vec4(+0.4,-0.4, 0.0, 0.0));
    EmitVertex();

    gl_Position = matrix * (gl_in[0].gl_Position + vec4(+0.4,+0.4, 0.0, 0.0));
    EmitVertex();

    EndPrimitive();
}
