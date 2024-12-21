#version 450

layout (location = 0) out vec4 fragc;

layout (location = 0) in vec3 frag;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 fragp;
layout (location = 3) in float time;
layout (location = 4) in vec2 uv;

vec3 lightPosition = vec3(10000.0);
vec3 color = vec3(1.0);

void main() {

    float ambientStrength = 0.2;
    vec3 ambientColor = color * ambientStrength;

    vec3 n = normalize(normal);
    vec3 lightDirection = normalize(lightPosition - fragp);
    vec3 diff = max(dot(n, lightDirection), 0.0) * vec3(1.0);

    fragc = vec4(vec2(uv), 0.0, 1.0);
}
