#version 400

uniform vec4 color;
in float angle;

void main(void)
{
    gl_FragColor = angle * color;
}
