#version 460
#extension GL_EXT_descriptor_heap : require
#extension GL_EXT_nonuniform_qualifier : require

struct VSOut
{
    vec3 pos;
    vec3 color;
    vec2 texCoord;
};
layout(location = 0) in VSOut fragIn;
layout(location = 0) out vec4 fragColor;

layout(descriptor_heap) uniform texture2D textures[];
layout(descriptor_heap) uniform sampler samplers[];

void main()
{
    fragColor = texture(sampler2D(textures[3024], samplers[2048]), vec2(0.5, 0.5));
}