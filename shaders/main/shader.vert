#version 450

struct anopolStandardPushConstants {
    vec4 scale;
    vec4 position;
    vec4 rotation;
    vec4 color;
    mat4 model;
    int instanced;
    int batched;
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

layout(std140, binding = 3) buffer BatchingTransformation {
    mat4 modelMatrix[];
} transformationBuffer;

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
        normal  = normalize(transpose(inverse(mat3(pushConstants.object.model))) * inNormal);
        fragp   = (pushConstants.object.model * vec4(inVertex, 1.0)).xyz;
        return;
    }

    if (pushConstants.object.batched == 1 && pushConstants.object.instanced == 0) {
        gl_Position = ubo.projection * ubo.lookAt * transformationBuffer.modelMatrix[0] * vec4(inVertex, 1.0);
        normal  = normalize(transpose(inverse(mat3(transformationBuffer.modelMatrix[0]))) * inNormal);
        fragp   = (pushConstants.object.model * vec4(inVertex, 1.0)).xyz;
        frag    = vec3(1.0);
        return;
    }

    if (pushConstants.object.instanced == 1 && pushConstants.object.batched == 0) {
        gl_Position = ubo.projection * ubo.lookAt * properties[gl_InstanceIndex].model * vec4(inVertex, 1.0);
        normal  = normalize(transpose(inverse(mat3(properties[gl_InstanceIndex].model))) * inNormal);
        fragp   = (properties[gl_InstanceIndex].model * vec4(inVertex, 1.0)).xyz;
        frag    = properties[gl_InstanceIndex].color.rgb;
        return;
    }
}
