#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 color;

layout(location = 0) out vec3 pos;
layout(location = 1) out vec3 norm;
layout(location = 2) out vec2 fuv;
layout(location = 3) out vec3 col;


layout(set = 0, binding = 4) uniform Uniforms {
    mat4 projection;
    mat4 view;
    mat4 model;
};

void main() {
    gl_Position = projection * view * model*vec4(position,1.0);
    pos = vec3(model*vec4(position,1.0));
    norm = vec3(inverse(transpose(model))*vec4(normal,1.0));
    fuv = uv;
    col = color;
}