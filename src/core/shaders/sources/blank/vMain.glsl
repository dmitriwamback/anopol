#version 410 core

layout(location=0) in vec3 vertex;
layout(location=1) in vec3 normal;
layout(location=2) in vec3 uv;
uniform mat4 projection;
uniform mat4 view;
uniform sampler2D background;
uniform sampler2D heightMap;

uniform float offset = 0.0;

uniform vec3 position;

out VERTEX {
    vec3 fragp;
    vec2 uv;
    vec3 absolutep;
    vec3 normal;
    float radius;
    vec2 offset;
    float zoom;
} o;

void main() {

    float radius = 1.905;
    o.absolutep = position;

    vec2 corner = o.absolutep.xz - vec2(radius);
    float u = -abs((vertex.x - corner.x) / (2 * radius));
    float v = abs((vertex.z - corner.y) / (2 * radius));

    o.uv = vec2(u - 0.5, v + 0.5);
    o.fragp = vertex + position;
    o.radius = radius;
    o.normal = cross(normal, uv);
    o.offset = vec2(0.1 + sin(offset*2), 0.1) * o.zoom;
    o.zoom = 1.1;

    float y = (texture(heightMap, (o.uv + o.offset)/o.zoom) / 1.5).r;
    gl_Position = projection * view * vec4(vec3(vertex.x, vertex.y + y/2.0, vertex.z) + position, 1.0);
}