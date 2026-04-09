#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord;

struct VSOut
{
    vec3 pos;
    vec3 color;
    vec2 texCoord;
};

layout(location = 0) out VSOut fragIn;


void main()
{
    fragIn.pos   = aPos;
    gl_Position = vec4(aPos, 1.0);
    fragIn.color = aColor;
    fragIn.texCoord = aTexCoord;
}
