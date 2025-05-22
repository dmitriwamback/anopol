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
    int batched;
    int physicallyBasedRendering;
};
layout (push_constant) uniform PushConstant {
    anopolStandardPushConstants object;
} pushConstants;

layout(set = 1, binding = 0) uniform sampler2D baseTexture;


layout (std140, binding = 2) uniform anopolStandardUniform {
    mat4 projection;
    mat4 lookAt;

    vec3 cameraPosition;
    float t;
    float fogDst;
} ubo;

vec3 lightPosition = vec3(10000.0, 10000.0, 10000.0);
vec3 lightColor = vec3(243, 165, 90)/255.0;
vec3 color = vec3(1.0);

vec3 fogColor = vec3(0.4, 0.7, 1.0);
float fogdst = ubo.fogDst;

vec3 applyFog(vec3 color, float distance) {
    float fogFactor = clamp(exp(-distance / fogdst), 0.0, 1.0);
    return mix(fogColor, color, fogFactor);
}

void main() {

    if (pushConstants.object.instanced == 1 && pushConstants.object.batched == 0) { 
        color = frag; 
    }
    else if (pushConstants.object.instanced == 0 && pushConstants.object.batched == 1) {
        color = frag;
    }
    else {
        color = pushConstants.object.color.rgb;
    }

    if (pushConstants.object.physicallyBasedRendering == 0) {
        float ambientStrength = 0.2;
        vec3 ambientColor = frag * ambientStrength;

        vec3 n = normalize(normal);
        vec3 an = abs(n);
        vec3 lightDirection = normalize(lightPosition - fragp);
        vec3 viewDirection = normalize(cameraPosition - fragp);

        vec3 diff = max(dot(n, lightDirection), 0.0) * color;

        vec3 halfWay = normalize(lightDirection + viewDirection);
        vec3 reflectDirection = reflect(-lightDirection, n);

        float spec = pow(max(dot(n, halfWay), 0.0), 8.0);
        vec3 specular = lightColor * spec;

        fragc = texture(baseTexture, uv) * vec4(diff + specular + ambientColor, 1.0);
    }
    else {

    }


    float gamma = 2.1;
    fragc.rgb = pow(fragc.rgb, vec3(1.0/gamma));
    fragc.rgb = applyFog(fragc.rgb, length(fragp - cameraPosition));
}
