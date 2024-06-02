#version 410 core
out vec4 fragc;
uniform float opacity;
uniform sampler2D background;
uniform sampler2D normalmap;
uniform sampler2D shadowMap;
uniform int hasTexture;

uniform vec3 viewPosition;

in VERTEX {
    vec3 fragp;
    vec2 uv;
    vec4 fragpl;
    mat3 TBN;
} i;
vec3 normal = vec3(0.0, 1.0, 0.0);

float computeShadow() {
    vec3 proj = i.fragpl.xyz / i.fragpl.w;
    proj = proj * 0.5 + 0.5;

    float closest = texture(shadowMap, proj.xy).r;
    float current = proj.z;
    float bias = 0.00005;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, proj.xy + vec2(x, y) * texelSize).r; 
            shadow += current - bias > pcfDepth ? 0.5 : 0.0;        
        }    
    }
    shadow /= 9.0;

    return shadow;
}

void main() {

    vec3 lightPosition = vec3(1, 1, 1);

    vec3 tangentfragp = i.TBN * i.fragp;
    vec3 tangentlightposition = i.TBN * lightPosition;
    vec3 tangentviewposition = i.TBN * viewPosition;

    vec3 color = vec3(0.7);

    vec3 n = texture(normalmap, abs(i.uv * 2)).rgb;
    n = normalize(n * 2.0 - 1.0);

    vec3 lightDirection = normalize(lightPosition - i.fragp);
    vec3 viewDirection  = normalize(viewPosition - i.fragp);
    vec3 reflectDirection = reflect(-lightDirection, n);
    vec3 halfWay = normalize(lightDirection + viewDirection);

    float diff = max(dot(lightDirection, n), 0.0);
    vec3 diffuse = diff * color;

    float spec = pow(max(dot(n, halfWay), 1.0), 36.0);
    vec3 specular = vec3(0.2) * spec;

    vec4 col = vec4(diffuse + specular + color * 0.1, 1.0);
    vec3 desaturatedcol = vec3((col.x + col.y + col.z) / 3.0);

    vec3 post = mix(desaturatedcol, col.rgb, 0.2);

    float gamma = 0.8;
    vec3 finalcol = pow(post, vec3(1.0 / gamma));

    float samplecol = (finalcol.r + finalcol.g + finalcol.b)/3.0;
    finalcol = texture(background, i.uv).rgb * 1.5;

    fragc = vec4(finalcol * (1.0 - computeShadow()), 1.0);
    //fragc = vec4(1.0, 1.0, 1.0, 1.0);
    //fragc = vec4(i.normal, 0.0, 1.0);
    //fragc = texture(shadowMap, i.uv);
}