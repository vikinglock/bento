#version 330 core
#ifdef GL_ARB_shading_language_420pack
#extension GL_ARB_shading_language_420pack : require
#endif
uniform vec3 color;
uniform sampler2D tex;
in vec3 norm;
in vec2 fuv;
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 twoColor;
in vec3 pos;
in vec3 col;
void main()
{
    float mult = 0.5 + (0.5 * dot(vec3(0.0, 1.0, 0.0), norm));
    vec4 scol = texture(tex, fuv);
    outColor = vec4(color.x * fuv.y, color.y * fuv.x, color.z * mult, 1.0);
    twoColor = vec4(scol.x * fuv.y, scol.y * fuv.x, scol.z * mult, scol.w);
}