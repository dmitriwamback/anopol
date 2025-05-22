#version 450

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

struct instanceProperties {
    mat4 model;
    vec4 color;
};

struct batchingTransformation {
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
    float fogDst;
} ubo;

layout(std140, binding = 3) readonly buffer BatchingTransformation {
    batchingTransformation batch[];
};

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
    
    if (pushConstants.object.instanced == 0 && pushConstants.object.batched == 0) {
        gl_Position = ubo.projection * ubo.lookAt * pushConstants.object.model * vec4(inVertex, 1.0);
        normal  = normalize(transpose(inverse(mat3(pushConstants.object.model))) * inNormal);
        fragp   = (pushConstants.object.model * vec4(inVertex, 1.0)).xyz;
        return;
    }

    if (pushConstants.object.batched == 1 && pushConstants.object.instanced == 0) {

        batchingTransformation currentBatch = batch[gl_InstanceIndex];
        mat4 model = currentBatch.model;
        vec3 color = currentBatch.color.rgb;

        gl_Position = ubo.projection * ubo.lookAt * model * vec4(inVertex, 1.0);
        normal  = normalize(transpose(inverse(mat3(model))) * inNormal);
        fragp   = (model * vec4(inVertex, 1.0)).xyz;
        frag    = color;
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
