#version 410 core

layout(location=0) in vec3 vertex;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;
uniform mat4 projection;
uniform mat4 view;

uniform mat4 lightprojection;
uniform mat4 lightlookat;

uniform vec3 position;

out VERTEX {
    vec3 fragp;
    vec4 fragpl;
    vec3 normal;
    vec2 uv;
    vec2 g_uv;
} o;

void main() {
    o.uv = (vertex.xz) / 2;
    o.g_uv = uv;
    o.fragp = vertex + position;
    o.normal = normalize(normal);

    o.fragpl = lightprojection * lightlookat * vec4(vertex + position, 1.0);
    gl_PointSize = 10.0;

    gl_Position = projection * view * vec4(vertex + position, 1.0);
}