#version 400

uniform vec4 colorA;
uniform vec4 colorB;
uniform vec4 light_colorD;
uniform vec4 light_colorA;
uniform vec4 light_colorS;

in vec4 n;
in vec4 l;
in vec4 e;

void main(void)
{
    vec4 r = reflect(-l, n);
    float cosAlpha = clamp(dot(e,r), 0, 1);
    float cosTheta = clamp(dot(n,l), 0, 1);

    vec4 ambient  = light_colorA * colorA;
    vec4 diffuse  = cosTheta * colorA * light_colorD;
    vec4 specular = vec4(0.5, 0.5, 0.5, 1.0) * light_colorS * pow(cosAlpha, 5);

    gl_FragColor = ambient + diffuse + specular;
}
