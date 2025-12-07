#version 330
#ifdef GL_ARB_shading_language_420pack
#extension GL_ARB_shading_language_420pack : require
#endif

layout(binding = 4, std140) uniform Uniforms
{
    float strength;
    float exposure;
} _44;

layout(binding = 0) uniform sampler2D diftex;
layout(binding = 1) uniform sampler2D blurtex;

in vec2 fUv;
layout(location = 1) out vec4 fragColor;
layout(location = 0) out float depth;

void main()
{
    vec2 uv = vec2(fUv.x, 1.0 - fUv.y);
    vec4 color = texture(diftex, uv);
    vec4 blurredColor = texture(blurtex, uv);
    fragColor = (color + (blurredColor * _44.strength)) * _44.exposure;
    depth = 0.0;
}

