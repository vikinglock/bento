#version 330
#ifdef GL_ARB_shading_language_420pack
#extension GL_ARB_shading_language_420pack : require
#endif

layout(binding = 4, std140) uniform Uniforms
{
    float threshold;
} _49;

layout(binding = 0) uniform sampler2D diftex;

in vec2 fUv;
layout(location = 1) out vec4 fragColor;
layout(location = 0) out float depth;

void main()
{
    vec2 uv = vec2(fUv.x, 1.0 - fUv.y);
    vec3 color = texture(diftex, uv).xyz;
    if ((((color.x + color.y) + color.z) / 3.0) > _49.threshold)
    {
        fragColor = vec4(color, 1.0);
    }
    else
    {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    depth = 0.0;
}

