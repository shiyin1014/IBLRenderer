#version 330 core

layout(location = 0) out vec4 FragColor;        // colorBuffer of scene
layout(location = 2) out vec4 DepthColor;       // depthBuffer of scene

// depth
uniform float near;
uniform float far;

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosFromLightSpace;

// material parameters
uniform sampler2D texture_diffuse_1;        // 0
uniform sampler2D texture_normal_1;         // 1
uniform sampler2D texture_metalness_1;      // 2
uniform sampler2D texture_roughness_1;      // 3
uniform sampler2D texture_AO_1;             // 4
uniform sampler2D texture_emission_1;       // 5

// ibl
uniform samplerCube irradianceMap;          // 6
uniform samplerCube prefilterMap;           // 7
uniform sampler2D brdfLUTMap;               // 8

// lights : there is only 1 point light
uniform vec3 lightPositions[1];
uniform vec3 lightColors[1];

// camera
uniform vec3 camPos;

const float PI = 3.14159265359;

// hdr and gamma
uniform int HDRToneMapping;
uniform int gammaCorrection;

// shadow
uniform bool hasShadow;
uniform sampler2D depthMap;                 // 9
uniform int shadowMethod;
float bias;
#define WEIGHT_OF_LIGHT 20.0f
#define NUM_SAMPLES 25
#define NUM_RINGS 10
#define PI 3.141592653589793
#define PI2 6.283185307179586
vec2 possionDisk[NUM_SAMPLES];


// linearDepth
float LinearizeDepth(float depth){
    float z = depth * 2.0f - 1.0f; // back to NDC
    return (2.0f * near * far) / (far + near - z * (far - near));
}

// ----------------------------------------------------------------------------
vec3 getNormalFromMap()
{
    // get normal in tangent space
    vec3 tangentNormal = texture(texture_normal_1, TexCoords).xyz * 2.0 - 1.0;

    // translate normal from tangent space to world space
    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

//-----------------------------------------------------------------------------
// shadowMapping
float ShadowMapping(){
    vec3 projectionCoords = FragPosFromLightSpace.xyz / FragPosFromLightSpace.w;    // from clip-space to ndc
    vec3 screenCoords = projectionCoords * 0.5f + 0.5f;      // from ndc to screen
    float currentDepth = screenCoords.z;    // depth from point light position

    if(currentDepth > 1.0f){
        return 0.0f;
    }

    // sample from depthMap to get closestDepth
    float closestDepth = texture(depthMap, screenCoords.xy).r;

    if(currentDepth - bias > closestDepth){
        return 1.0f;
    }

    return 0.0f;
}

//-----------------------------------------------------------------------------
// PCF
float PCF(){
    vec3 ndcCoords = FragPosFromLightSpace.xyz / FragPosFromLightSpace.w;
    vec3 screenCoords = ndcCoords * 0.5f + 0.5f;
    float currentDepth = screenCoords.z;

    if(currentDepth > 1.0f) return 0.0f;

    // 5*5 filter size
    float shadow = 0.0f;
    vec2 texelSize = 1.0f / textureSize(depthMap,0);
    for(int i = -2; i <= 2; i++){
        for(int j = -2; j <= 2; j++){
            float nearest_depth = texture(depthMap, screenCoords.xy + vec2(i,j) * texelSize).r;
            shadow += currentDepth - bias > nearest_depth ? 1.0f : 0.0f;
        }
    }
    return shadow / NUM_SAMPLES;
}

//-----------------------------------------------------------------------------
// random function
float rand_2to1(vec2 uv) {
    // 0 - 1
    float a = 12.9898, b = 78.233, c = 43758.5453;
    float dt = dot( uv.xy, vec2( a,b ) );
    float sn = mod(dt, PI);
    return fract(sin(sn) * c);
}

//-----------------------------------------------------------------------------
// possion disk sample randomly
void possionDiskSamples(vec2 randomSeed){

    float ANGLE_STEP = PI2 * float( NUM_RINGS ) / float( NUM_SAMPLES );
    float INV_NUM_SAMPLES = 1.0 / float( NUM_SAMPLES );

    float angle = rand_2to1( randomSeed ) * PI2;
    float radius = INV_NUM_SAMPLES;
    float radiusStep = radius;

    for( int i = 0; i < NUM_SAMPLES; i ++ ) {
        possionDisk[i] = vec2( cos( angle ), sin( angle ) ) * pow( radius, 0.75 );
        radius += radiusStep;
        angle += ANGLE_STEP;
    }
}

//-----------------------------------------------------------------------------
// fina a avgDepth of blocker
float findBlockerDepth(float current_depth, vec2 current_uv){
    possionDiskSamples(current_uv);
    float count = 0.0f;
    vec2 texelSize = 1.0f / textureSize(depthMap,0);
    float total_depth = 0.0f;
    for(int i = 0; i < NUM_SAMPLES; i++){
        float sample_depth = texture(depthMap, current_uv + possionDisk[i] * texelSize).r;
        if(current_depth - bias > sample_depth){
            count += 1.0f;
            total_depth += sample_depth;
        }
    }
    if(count == 0.0f) return 1.0f;
    return total_depth / count;
}

//-----------------------------------------------------------------------------
// PCSS
float PCSS(){
    vec3 ndcCoords = FragPosFromLightSpace.xyz / FragPosFromLightSpace.w;
    vec3 screenCoords = ndcCoords * 0.5f + 0.5f;
    float currentDepth = screenCoords.z;
    float closestDepth = texture(depthMap, screenCoords.xy).r;

    if(currentDepth > 1.0f) return 0.0f;

    // step1 : avgBlocker depth
    float avgDepth = findBlockerDepth(currentDepth, screenCoords.xy);
    // step2 : penumbra depth
    float penumbra = max((currentDepth - avgDepth),0.0f) * WEIGHT_OF_LIGHT / avgDepth;
    // step3 : PCF
    float shadow = 0.0f;
    vec2 texelSize = 1.0f / textureSize(depthMap,0);
    for(int i = 0; i< NUM_SAMPLES; i++){
        vec2 sample_uv = screenCoords.xy + possionDisk[i] * texelSize * penumbra;
        float sample_depth = texture(depthMap, sample_uv).r;
        if(currentDepth - bias > sample_depth){
            shadow += 1.0f;
        }
    }
    return shadow / float(NUM_SAMPLES);
}

void main(){

    // material properties
    vec3 albedo = pow(texture(texture_diffuse_1, TexCoords).rgb, vec3(2.2));
    float metallic = texture(texture_metalness_1, TexCoords).r;
    float roughness = texture(texture_roughness_1, TexCoords).r;
    float ao = texture(texture_AO_1, TexCoords).r;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(camPos - WorldPos);
    vec3 R = reflect(-V, N);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    float shadow = 0.0f;
    for(int i = 0; i < 1; ++i)      // note that we have only 1 point light.
    {
        // calculate per-light radiance
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        // bias
        bias = max(0.005f * (1.0f - dot(normalize(N),L)),0.005f);

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;

        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);

        // calculate shadow
        if(hasShadow){
            if(shadowMethod == 1){
                shadow = ShadowMapping();
            }else if(shadowMethod == 2){
                shadow = PCF();
            }else if(shadowMethod == 3){
                shadow = PCSS();
            }
        }

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    // ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse  = irradiance * albedo;                        // 漫反射

    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;   // 采样预滤波环境贴图
    vec2 brdf  = texture(brdfLUTMap, vec2(max(dot(N, V), 0.0), roughness)).rg;                  // 采样 BRDF LUT
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y); // 两个积分相乘,注意这里用了F来近似F0

    vec3 ambient = (kD * diffuse + specular) * ao;

    vec3 emission = texture(texture_emission_1,TexCoords).rgb;

    // shadow term
    vec3 color = vec3(0.0f);

    if(shadow == 0.0f){
        color = ambient + Lo + emission;
    }else{
        // for rendering shadow in IBL(point light only), we apply a scale term to ambient and a shadow term to ambient and Lo.
        color = ambient * 0.1f + (1.0f - shadow) * (ambient * 0.9f + Lo) + emission;
    }

    // HDR tonemapping
    if(HDRToneMapping == 1){
        color = color / (color + vec3(1.0f));
    }
    // gamma correct
    if(gammaCorrection == 1){
        color = pow(color, vec3(1.0f/2.2f));
    }

    FragColor = vec4(color , 1.0f);

    // store depthBuffer from camera (for display)
    float depth = LinearizeDepth(gl_FragCoord.z) / far;
    DepthColor = vec4(vec3(depth),1.0f);
}
