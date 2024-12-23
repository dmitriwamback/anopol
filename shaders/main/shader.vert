#version 450

struct anopolStandardPushConstants {
    vec4 scale;
    vec4 position;
    vec4 rotation;
    mat4 model;
    int instanced;
};

struct instanceProperties {
    mat4 model;
    vec4 color;
};

layout (push_constant, std430) uniform PushConstant {
    anopolStandardPushConstants object;
} pushConstants;

layout (std140, binding = 1) readonly buffer InstanceBuffer {
    instanceProperties properties[];
};


layout (std140, binding = 2) uniform anopolStandardUniform {
    mat4 projection;
    mat4 lookAt;

    vec3 cameraPosition;
    float t;
} ubo;

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
    uv      = UV;
    cameraPosition = ubo.cameraPosition;
    
    if (pushConstants.object.instanced == 0) {
        gl_Position = ubo.projection * ubo.lookAt * pushConstants.object.model * vec4(inVertex, 1.0);
        normal  = mat3(transpose(inverse(pushConstants.object.model))) * inNormal;
        fragp   = (pushConstants.object.model * vec4(inVertex, 1.0)).xyz;
    }
    else {
        gl_Position = ubo.projection * ubo.lookAt * properties[gl_InstanceIndex].model * vec4(inVertex, 1.0);
        normal  = mat3(transpose(inverse(properties[gl_InstanceIndex].model))) * inNormal;
        fragp   = (properties[gl_InstanceIndex].model * vec4(inVertex, 1.0)).xyz;
    }
}
