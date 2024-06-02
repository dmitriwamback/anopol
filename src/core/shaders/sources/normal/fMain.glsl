#version 410 core
out vec4 fragc;
uniform float opacity;
uniform sampler2D background;
uniform sampler2D normalmap;
uniform int hasTexture;
uniform sampler2D shadowMap;

uniform vec3 viewPosition;
uniform vec3 sun_position;

in VERTEX {
    vec3 fragp;
    vec4 fragpl;
    vec3 normal;
    vec2 uv;
    vec2 g_uv;
} i;


float computeShadow(vec3 lightdirection) {
    vec3 proj = i.fragpl.xyz / i.fragpl.w;
    proj = proj * 0.5 + 0.5;

    float closest = texture(shadowMap, proj.xy).r;
    float current = proj.z;
    //float bias = 0.000005;
    float bias = min(0.000001 * (1.0 - dot(i.normal, lightdirection)), 0.00001);

    float shadow = current + bias > closest ? 0.5 : 0.0;
    return shadow;
}

void main() {

    vec3 lightPosition = sun_position * 1000;

    vec3 lightDirection = normalize(lightPosition - i.fragp);

    float shadow = computeShadow(lightDirection);
    float full_shadow = 1.0 - shadow;
    float half_shadow = shadow + 0.5;


    vec3 viewDirection  = normalize(viewPosition - i.fragp);
    vec3 reflectDirection = reflect(-lightDirection, i.normal);
    vec3 halfWay = normalize(lightDirection + viewDirection);

    float diff = max(dot(lightDirection, i.normal), 0.0) * (1 - half_shadow);
    if (dot(lightDirection, i.normal) <= 0.05) {
        diff = 0;
    }
    vec3 diffuse = diff * texture(background, i.g_uv).rgb;

    float spec = pow(max(dot(i.normal, halfWay), 1.0), 3.0);
    vec3 specular = vec3(0.1) * 0;

    vec2 uv = i.uv;

    vec4 col = vec4((diffuse + specular) * half_shadow + texture(background, i.g_uv).rgb * vec3(0.6, 0.6, 0.8), 1.0);

    fragc = vec4(col.rgb * (1 - computeShadow(lightDirection)), 1);
    //fragc = vec4(vec3(1.0) * (1 - computeShadow(lightDirection)), 1.0);
    //fragc = vec4(i.g_uv, 0.0, 1.0);
}