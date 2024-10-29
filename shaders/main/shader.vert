#version 450

layout (binding = 0) uniform anopolStandardUniform {
    mat4 projection;
    mat4 lookAt;
    float t;
} ubo;

layout (location = 0) in vec3 inVertex;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 UV;

layout (location = 0) out vec3 frag;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec3 fragp;
layout (location = 3) out float time;

void main() {
    
    time    = ubo.t/2;
    frag    = vec3(1.0);
    normal  = inNormal;
    fragp   = inVertex;
    
    gl_Position = ubo.projection * ubo.lookAt * vec4(inVertex, 1.0);
}
