#version 440

layout (binding = 0) uniform anopolStandardUniform {
    float t;
} ubo;

layout (location = 0) in vec3 inVertex;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 UV;

layout (location = 0) out vec3 frag;
layout (location = 1) out float time;

void main() {
    
    frag = inNormal;
    time = ubo.t;
    
    gl_Position = vec4(inVertex.xy, 0, 1.0);
}
