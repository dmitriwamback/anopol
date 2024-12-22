#version 450

layout (location = 0) out vec4 fragc;

layout (location = 0) in vec3 frag;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 fragp;
layout (location = 3) in float time;
layout (location = 4) in vec2 uv;
layout (location = 5) in vec3 cameraPosition;

vec3 lightPosition = vec3(0.0, 4.0, 0.0);
vec3 lightColor = vec3(1.0, 0.0, 1.0);
vec3 color = vec3(1.0);

void main() {

    float ambientStrength = 0.2;
    vec3 ambientColor = color * ambientStrength;

    vec3 n = normalize(normal);
    vec3 lightDirection = normalize(lightPosition - fragp);
    vec3 viewDirection = normalize(cameraPosition - fragp);

    vec3 diff = max(dot(n, lightDirection), 0.0) * vec3(1.0);

    vec3 halfWay = normalize(lightDirection + viewDirection);
    vec3 reflectDirection = reflect(-lightDirection, n);

    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 8.0);
    vec3 specular = lightColor * spec;

    fragc = vec4(diff + specular + ambientColor, 1.0);
}
