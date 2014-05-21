#version 400

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 14) out;

uniform float pointSize;
uniform mat4 matrix;
out float angle;

void main(void)
{
    float shift = pointSize / 512.0;

    vec4 pos1 = matrix * (gl_in[0].gl_Position + vec4(+shift,+shift,-shift, 0.0)); //1
    vec4 pos2 = matrix * (gl_in[0].gl_Position + vec4(-shift,+shift,-shift, 0.0)); //2
    vec4 pos3 = matrix * (gl_in[0].gl_Position + vec4(+shift,+shift,+shift, 0.0)); //3
    vec4 pos4 = matrix * (gl_in[0].gl_Position + vec4(-shift,+shift,+shift, 0.0)); //4
    vec4 pos5 = matrix * (gl_in[0].gl_Position + vec4(+shift,-shift,-shift, 0.0)); //5
    vec4 pos6 = matrix * (gl_in[0].gl_Position + vec4(-shift,-shift,-shift, 0.0)); //6
    vec4 pos7 = matrix * (gl_in[0].gl_Position + vec4(-shift,-shift,+shift, 0.0)); //7
    vec4 pos8 = matrix * (gl_in[0].gl_Position + vec4(+shift,-shift,+shift, 0.0)); //8

    float face_t = 1.0;
    float face_b = 0.5;

    float diag_f = 1.0;
    float diag_m = 0.8;
    float diag_b = 0.5;

    float a1 = face_b * diag_m;
    float a2 = face_b * diag_b;
    float a3 = face_t * diag_m;
    float a4 = face_t * diag_b;
    float a5 = face_b * diag_f;
    float a6 = face_b * diag_m;
    float a7 = face_t * diag_m;
    float a8 = face_t * diag_f;

    gl_Position = pos4; angle = a4; EmitVertex(); // T
    gl_Position = pos3; angle = a3; EmitVertex(); // T
    gl_Position = pos7; angle = a7; EmitVertex(); // T
    gl_Position = pos8; angle = a8; EmitVertex(); // T
    gl_Position = pos5; angle = a5; EmitVertex(); // B
    gl_Position = pos3; angle = a3; EmitVertex(); // T
    gl_Position = pos1; angle = a1; EmitVertex(); // B
    gl_Position = pos4; angle = a4; EmitVertex(); // T
    gl_Position = pos2; angle = a2; EmitVertex(); // B
    gl_Position = pos7; angle = a7; EmitVertex(); // T
    gl_Position = pos6; angle = a6; EmitVertex(); // B
    gl_Position = pos5; angle = a5; EmitVertex(); // B
    gl_Position = pos2; angle = a2; EmitVertex(); // B
    gl_Position = pos1; angle = a1; EmitVertex(); // B

    EndPrimitive();
}
