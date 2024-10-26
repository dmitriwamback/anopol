#version 440

layout (location = 0) out vec4 fragp;

layout (location = 0) in vec3 frag;

void main() {
    fragp = vec4(frag, 1.0);
}
