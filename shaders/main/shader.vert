#version 450

struct anopolStandardPushConstants {
    vec4 scale;
    vec4 position;
    vec4 rotation;
    mat4 model;
};

layout (binding = 0) uniform anopolStandardUniform {
    mat4 projection;
    mat4 lookAt;

    vec3 cameraPosition;
    float t;
} ubo;

layout (push_constant) uniform PushConstant {
    anopolStandardPushConstants object;
} pushConstants;

layout (location = 0) in vec3 inVertex;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 UV;

layout (location = 0) out vec3 frag;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec3 fragp;
layout (location = 3) out float time;

layout (location = 4) out vec2 uv;
layout (location = 5) out vec3 cameraPosition;

void main() {
    
    time    = ubo.t/2;
    frag    = vec3(1.0);
    normal  = mat3(transpose(inverse(pushConstants.object.model))) * inNormal;
    fragp   = (pushConstants.object.model * vec4(inVertex, 1.0)).xyz;
    uv      = UV;
    cameraPosition = ubo.cameraPosition;
    
    gl_Position = ubo.projection * ubo.lookAt * pushConstants.object.model * vec4(inVertex, 1.0);
}
