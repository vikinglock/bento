#version 330
#ifdef GL_ARB_shading_language_420pack
#extension GL_ARB_shading_language_420pack : require
#endif

struct Light
{
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

const float _229[64] = float[](0.0, 0.5, 0.125, 0.625, 0.03125, 0.53125, 0.15625, 0.65625, 0.75, 0.25, 0.875, 0.375, 0.78125, 0.28125, 0.90625, 0.40625, 0.1875, 0.6875, 0.0625, 0.5625, 0.21875, 0.71875, 0.09375, 0.59375, 0.9375, 0.4375, 0.8125, 0.3125, 0.96875, 0.46875, 0.84375, 0.34375, 0.046875, 0.546875, 0.171875, 0.671875, 0.015625, 0.515625, 0.140625, 0.640625, 0.796875, 0.296875, 0.921875, 0.421875, 0.765625, 0.265625, 0.890625, 0.390625, 0.234375, 0.734375, 0.109375, 0.609375, 0.203125, 0.703125, 0.078125, 0.578125, 0.984375, 0.484375, 0.859375, 0.359375, 0.953125, 0.453125, 0.828125, 0.328125);

layout(binding = 4, std140) uniform Uniforms
{
    vec3 pos;
    int numLights;
    vec3 ambientColor;
    vec3 positions[50];
    float constants[50];
    float linears[50];
    float quadratics[50];
    vec3 ambients[50];
    vec3 diffuses[50];
    vec3 speculars[50];
    float tspecular;
} _62;

layout(binding = 0) uniform sampler2D diftex;
layout(binding = 1) uniform sampler2D nortex;
layout(binding = 2) uniform sampler2D depthtex;
layout(binding = 3) uniform sampler2D postex;

in vec2 fUv;
layout(location = 1) out vec4 fragColor;
layout(location = 0) out float depth;
vec3 fragPos;
vec3 fragNorm;
vec3 viewDir;

vec3 calculateLighting(Light light)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(fragNorm, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, fragNorm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), _62.tspecular);
    float _distance = length(light.position - fragPos);
    float attenuation = 1.0 / ((light.constant + (light.linear * _distance)) + (light.quadratic * (_distance * _distance)));
    vec3 text = vec3(texture(diftex, fUv).xyz);
    vec3 ambient = light.ambient * text;
    vec3 diffuse = (light.diffuse * diff) * text;
    vec3 specular = (light.specular * spec) * text;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse) + specular;
}

float dither(vec2 p, float a)
{
    int index = (int(mod(p.y, 8.0)) * 8) + int(mod(p.x, 8.0));
    float threshold = _229[index];
    return float(a > threshold);
}

void main()
{
    float t = 0.4000000059604644775390625;
    vec3 finalColor = vec3(0.100000001490116119384765625) + _62.ambientColor;
    fragNorm = normalize((texture(nortex, fUv).xyz * 2.0) - vec3(1.0));
    float fragDepth = texture(depthtex, fUv).x;
    fragPos = texture(postex, fUv).xyz;
    vec3 dif = texture(diftex, fUv).xyz;
    viewDir = normalize(_62.pos - fragPos);
    if (fragDepth > 0.0)
    {
        Light light;
        for (int i = 0; i < _62.numLights; i++)
        {
            light.position = _62.positions[i];
            light.ambient = _62.ambients[i];
            light.diffuse = _62.diffuses[i];
            light.specular = _62.speculars[i];
            light.constant = _62.constants[i];
            light.linear = _62.linears[i];
            light.quadratic = _62.quadratics[i];
            Light param = light;
            vec3 color = calculateLighting(param);
            vec2 param_1 = gl_FragCoord.xy;
            float param_2 = ((color.x + color.y) + color.z) / 3.0;
            if (dither(param_1, param_2) > 0.5)
            {
                finalColor += color;
            }
        }
    }
    fragColor = vec4(finalColor * texture(diftex, fUv).xyz, 1.0);
    depth = 0.0;
}

