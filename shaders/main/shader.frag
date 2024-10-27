#version 440

layout (location = 0) out vec4 fragp;

layout (location = 0) in vec3 frag;
layout (location = 1) in float time;

void main() {
    fragp = vec4(abs(frag) * (sin(time) + 1.0)/2.0, 1.0);
}
