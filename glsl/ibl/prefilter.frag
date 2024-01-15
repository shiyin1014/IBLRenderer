#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform float roughness;
uniform samplerCube environmentMap;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness){
    float a = roughness * roughness;
    float a2 = a * a;
    float nom = a2;

    float NdotH = max(dot(N,H),0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = NdotH2 * (a2 - 1.0f) + 1.0f;
    denom = PI * denom * denom;

    return nom / denom;
}

// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness*roughness;

    // 下面三行代码就体现了重要性采样的思想，逆变换采样 Inverse Transform Sampling
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

    // from spherical coordinates to cartesian coordinates - halfway vector
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // from tangent-space H vector to world-space sample vector
    vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

void main(){
    // 立方体片元的世界坐标作为局部的法向量
    vec3 N = normalize(WorldPos);

    // 为了简化，假设观察方向 V =  反射方向 R = 法向量 N
    vec3 V = N;

    // 围绕着N周围，根据粗糙度来采样光线的方向
    // 从上面的重要性采样的函数中也可以验证一点：当粗糙度为0的时候，采样的方向就是N本身
    const uint SAMPLE_COUNT = 1024u;        // 共计采样1024个
    vec3 prefilterColor = vec3(0.0f);       // 最终的颜色值
    float totalWeight = 0.0f;               // 存储权重

    for(uint i = 0u;i < SAMPLE_COUNT; i++){
        // 获得一个二维的随机数用于采样
        vec2 Xi = Hammersley(i,SAMPLE_COUNT);
        // 围绕着N周围，根据粗糙度生成一个采样向量作为半程向量H（如果粗糙度不是0，那么生成的H将不会是N，而是围绕着N周围的方向，粗糙度越大距离越远）
        // 因为有粗糙度参数的存在，越是光滑那么微表面的法向量越是接近宏观法线N，越是粗糙那么微表面的法向量越是远离宏观法线N
        // 这个得到的半程向量H实际上就是微表面的法线的一个样本方向N
        vec3 H = ImportanceSampleGGX(Xi,N,roughness);
        // 根据微表面的法向N，观察方向V，计算入射光线L
        vec3 L = normalize(2.0 * dot(V, H) * H - V);        // 等价于 normalize(reflect(-V,H));

        // 只有L和N在同一个半球内部才有效，否则计算的L无效
        float NdotL = max(dot(N, L), 0.0);
        // 对于每一次采样的情况，计算
        if(NdotL > 0.0){

            // 到这里其实已经可以直接采样环境贴图了，因为我们已经计算得到了光照的方向了
            //            prefilterColor += texture(environmentMap,L).rgb * NdotL;
            //            totalWeight += NdotL;
            // 但是这种方式会导致明亮区域周围出现点状图案

            // 下面使用 Chetan Jags 的方法来解决这个问题
            // 不是直接采样环境贴图，而是基于积分的pdf和粗糙度去采样环境贴图的mipmap
            float D   = DistributionGGX(N, H, roughness);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

            float resolution = 512.0; // resolution of source cubemap (per face)
            float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

            float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

            prefilterColor += textureLod(environmentMap, L, mipLevel).rgb * NdotL;
            totalWeight += NdotL;
        }
    }

    prefilterColor = prefilterColor / totalWeight;

    FragColor = vec4(prefilterColor, 1.0);
}