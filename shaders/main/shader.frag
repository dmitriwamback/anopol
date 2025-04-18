#version 450

layout (location = 0) out vec4 fragc;

layout (location = 0) in vec3 frag;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 fragp;
layout (location = 3) in float time;
layout (location = 4) in vec2 uv;
layout (location = 5) in vec3 cameraPosition;


struct anopolStandardPushConstants {
    vec4 scale;
    vec4 position;
    vec4 rotation;
    vec4 color;
    mat4 model;
    int instanced;
};
layout (push_constant) uniform PushConstant {
    anopolStandardPushConstants object;
} pushConstants;


vec3 lightPosition = vec3(10000.0, 10000.0, 10000.0);
vec3 lightColor = vec3(0.0, 1.0, 0.0);
vec3 color = vec3(1.0);

void main() {

    if (pushConstants.object.instanced == 1) { color = frag; }
    else {
        color = pushConstants.object.color.rgb;
    }

    float ambientStrength = 0.2;
    vec3 ambientColor = frag * ambientStrength;

    vec3 n = normalize(normal);
    vec3 lightDirection = normalize(lightPosition - fragp);
    vec3 viewDirection = normalize(cameraPosition - fragp);

    vec3 diff = max(dot(n, lightDirection), 0.0) * color;

    vec3 halfWay = normalize(lightDirection + viewDirection);
    vec3 reflectDirection = reflect(-lightDirection, n);

    float spec = pow(max(dot(n, halfWay), 0.0), 8.0);
    vec3 specular = lightColor * spec;

    vec2 xuv = fragp.zy * pushConstants.object.scale.x;
    vec2 yuv = fragp.xz * pushConstants.object.scale.y;
    vec2 zuv = fragp.xy * pushConstants.object.scale.z;

    fragc = vec4(diff + specular + ambientColor, 1.0);
}
