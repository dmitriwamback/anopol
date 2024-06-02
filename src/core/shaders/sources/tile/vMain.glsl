#version 410 core

layout(location=0) in vec3 vertex;
layout(location=1) in vec3 tangent;
layout(location=2) in vec3 bitangent;
uniform mat4 projection;
uniform mat4 view;

uniform mat4 lightprojection;
uniform mat4 lightlookat;

uniform vec3 position;

out VERTEX {
    vec3 fragp;
    vec2 uv;
    vec4 fragpl;
    mat3 TBN;
} o;

void main() {
    o.uv = (vertex.xz) / 2;
    o.fragp = vertex + position;
    o.TBN = mat3(
        normalize(tangent),
        normalize(bitangent),
        normalize(vec3(0, 1, 0))
    );
    o.fragpl = lightprojection * lightlookat * vec4(vertex + position, 1.0);
    gl_Position = projection * view * vec4(vertex + position, 1.0);
}