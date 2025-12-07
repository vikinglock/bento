#version 330 core
#ifdef GL_ARB_shading_language_420pack
#extension GL_ARB_shading_language_420pack : require
#endif
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
layout(location = 0) in vec3 position;
out vec3 pos;
out vec3 norm;
layout(location = 1) in vec3 normal;
out vec2 fuv;
layout(location = 2) in vec2 uv;
out vec3 col;
layout(location = 3) in vec3 color;
void main()
{
    gl_Position = ((projection * view) * model) * vec4(position, 1.0);
    pos = vec3((model * vec4(position, 1.0)).xyz);
    norm = vec3((inverse(transpose(model)) * vec4(normal, 1.0)).xyz);
    fuv = uv;
    col = color;
    gl_Position.z = 2.0 * gl_Position.z - gl_Position.w;
    gl_Position.y = -gl_Position.y;
}