#version 410 core
out vec4 fragc;
uniform float opacity = 1;
uniform sampler2D background;
uniform sampler2D normalmap;
uniform sampler2D heightMap;
uniform int hasTexture;

uniform vec3 viewPosition;
uniform int isHologram;
uniform int colorize;
uniform vec3 col = vec3(102, 178, 107);

in VERTEX {
    vec3 fragp;
    vec3 absolutep;
    vec2 uv;
    vec3 normal;
    float radius;
    vec2 offset;
    float zoom;
} i;
vec3 normal = vec3(0.0, 1.0, 0.0);

void main() {
    
    float y = texture(heightMap, ((i.uv + i.offset)/i.zoom) / 1.5).r * 4 + 1;
    vec3 color = vec3(col.r * max(y, 0.5), col.g, col.b / max(y, 0.5)) / 255.0;
    color = pow(color, vec3(2));

    if (distance(i.fragp.xz, i.absolutep.xz) > i.radius && isHologram == 1) discard;
    fragc = colorize == 1 ? vec4(mix(color, texture(background, ((i.uv + i.offset)/i.zoom) / 1.5).rgb, 1.0).xyz * y, opacity) : vec4(0.0, 0.0, 0.0, opacity);
    //fragc = vec4(i.uv, 0, 1.0);
}