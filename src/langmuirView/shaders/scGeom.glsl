#version 400

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

uniform float pointSize;
uniform mat4 matrix;
out float angle;

void main(void)
{
    float shift = pointSize / 512.0;

    gl_Position = matrix * (gl_in[0].gl_Position + vec4(-shift,-shift, 0.0, 0.0));
    angle = 0.6;
    EmitVertex();

    gl_Position = matrix * (gl_in[0].gl_Position + vec4(-shift,+shift, 0.0, 0.0));
    angle = 0.7;
    EmitVertex();

    gl_Position = matrix * (gl_in[0].gl_Position + vec4(+shift,-shift, 0.0, 0.0));
    angle = 0.8;
    EmitVertex();

    gl_Position = matrix * (gl_in[0].gl_Position + vec4(+shift,+shift, 0.0, 0.0));
    angle = 1.0;
    EmitVertex();

    EndPrimitive();
}
