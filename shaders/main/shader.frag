#version 450

layout (location = 0) out vec4 fragc;

layout (location = 0) in vec3 frag;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 fragp;
layout (location = 3) in float time;
layout (location = 4) in vec2 uv;
layout (location = 5) in vec3 cameraPosition;


struct anopolStandardPushConstants {
    vec4 scale;
    vec4 position;
    vec4 rotation;
    vec4 color;
    mat4 model;
    int instanced;
    int batched;
    int physicallyBasedRendering;
};
layout (push_constant) uniform PushConstant {
    anopolStandardPushConstants object;
} pushConstants;

layout(set = 1, binding = 0) uniform sampler2D baseTexture;
//layout(set = 1, binding = 1) uniform sampler2D metallic;
//layout(set = 1, binding = 2) uniform sampler2D roughness;
//layout(set = 1, binding = 3) uniform sampler2D normalMap;

layout (std140, binding = 2) uniform anopolStandardUniform {
    mat4 projection;
    mat4 lookAt;

    vec3 cameraPosition;
    float t;
    float fogDst;
} ubo;

vec3 lightPosition = vec3(0.0, 1000.0, 0.0);
vec3 lightColor = vec3(243, 165, 90)/255.0;
vec3 color = vec3(1.0);

vec3 fogColor = vec3(0.4, 0.7, 1.0);
float fogdst = ubo.fogDst;


float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14159265358 * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}




vec3 applyFog(vec3 col, float distance) {
    float fogFactor = clamp(exp(-distance / fogdst), 0.0, 1.0);
    return mix(fogColor, col, fogFactor);
}

void main() {

    if (pushConstants.object.instanced == 1 && pushConstants.object.batched == 0) { 
        color = frag; 
    }
    else if (pushConstants.object.instanced == 0 && pushConstants.object.batched == 1) {
        color = frag;
    }
    else {
        color = pushConstants.object.color.rgb;
    }

    vec3 n = normalize(normal);
    vec3 viewDirection = normalize(cameraPosition - fragp);

    float minDistance = 1.0;
    float maxDistance = 50.0;
    float cameraDst = length(cameraPosition - fragp);
    float lod = (log2(cameraDst) - log2(minDistance)) / (log2(maxDistance) - log2(minDistance)) * 9;
    lod = clamp(lod, 0.0, 9);

    vec3 lightDirection = normalize(lightPosition - fragp);

    vec4 _albedo = textureLod(baseTexture, uv, lod);
    vec3 albedo = _albedo.rgb;

    if (pushConstants.object.physicallyBasedRendering == 0) {
        float ambientStrength = 0.2;
        vec3 ambientColor = frag * ambientStrength;

        vec3 diff = max(dot(n, lightDirection), 0.0) * color;

        vec3 halfWay = normalize(lightDirection + viewDirection);
        vec3 reflectDirection = reflect(-lightDirection, n);

        float spec = pow(max(dot(n, halfWay), 0.0), 8.0);
        vec3 specular = lightColor * spec;

        fragc = _albedo * vec4(diff + specular + ambientColor, 1.0);
    }
    else {
        float mockMetallic = 0.25;
        float mockRoughness = albedo.r;

        vec3 F0 = vec3(0.04); 
        F0 = mix(F0, albedo, mockMetallic);

        vec3 Lo = vec3(0.0);

        vec3 H = normalize(viewDirection + lightDirection);
        float dst = length(lightPosition - fragp);
        float attenuation = 1.0 / (dst * dst);
        vec3 radiance = lightColor * attenuation * 10000000.0;

        float NDF = DistributionGGX(n, H, mockRoughness);   
        float G   = GeometrySmith(n, viewDirection, lightDirection, mockRoughness);      
        vec3 F    = fresnelSchlick(max(dot(H, viewDirection), 0.0), F0);
        
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(n, viewDirection), 0.0) * max(dot(n, lightDirection), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;
        
        vec3 kS = F;
        vec3 kD = clamp(vec3(1.0) - kS, 0.0, 1.0);

        kD *= 1 - mockMetallic;	  

        float NdotL = max(dot(n, lightDirection), 0.0);        

        Lo += (kD * albedo / 3.14159265358 + specular) * radiance * NdotL;

        vec3 ambient = vec3(0.2) * albedo;
        vec3 col = ambient + Lo;
        col = col / (col + vec3(1.0));
        fragc = vec4(col, 1.0);
    }


    float gamma = 2.1;
    fragc.rgb = pow(fragc.rgb, vec3(1.0/gamma));
    fragc.rgb = applyFog(fragc.rgb, length(fragp - cameraPosition));
}
