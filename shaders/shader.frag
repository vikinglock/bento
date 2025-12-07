#version 450

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 twoColor;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 fuv;
layout(location = 3) in vec3 col;

layout(set = 0,binding = 0) uniform sampler2D tex;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

layout(binding = 0)uniform Uniforms{
    vec3 color;
};

void main() {
    float mult = 0.5+0.5*dot(vec3(0,1,0),norm);
    //outColor = vec4(color.x*mult,color.y*mult,color.z*mult,1.0);
    vec4 scol = texture(tex,fuv);
    outColor = vec4(color.x*fuv.y,color.y*fuv.x,color.z*mult,1.0);
    twoColor = vec4(scol.x*fuv.y,scol.y*fuv.x,scol.z*mult,scol.w);
}