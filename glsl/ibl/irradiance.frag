#version 330 core
out vec4 FragColor;

in vec3 WorldPos;

uniform samplerCube envCubeMap;

const float PI = 3.14159265359;

void main(){

    vec3 N = WorldPos;

    vec3 irradiance = vec3(0.0f);

    // 以半球的法向量建立一个局部空间的坐标系
    vec3 up = vec3(0,1,0);
    vec3 right = normalize(cross(up,N));
    up = normalize(cross(N,right));

    float sampleDelta = 0.025;
    float number_of_sample = 0.0f;

    for(float phi = 0.0f; phi < 2.0f * PI; phi += sampleDelta){
        for(float theta = 0.0f; theta < 0.5 * PI; theta += sampleDelta){
            // 将局部球体坐标系转换为局部3维坐标系
            vec3 sphere_local_coord = vec3(sin(theta) * sin(phi), sin(theta) * cos(phi), cos(theta));
            // 将局部三维坐标系转换为世界坐标系
            vec3 three_d_world_coord = sphere_local_coord.x * right + sphere_local_coord.y * up + sphere_local_coord.z * N;

            // 采样
            irradiance += texture(envCubeMap,normalize(three_d_world_coord)).rgb * cos(theta) * sin(theta);
            number_of_sample ++;
        }
    }

    irradiance = PI * irradiance * (1.0f / number_of_sample);

    FragColor = vec4(irradiance,1.0f);
}