#version 330 core
out vec4 FragColor;

in vec3 WorldPos;

uniform samplerCube cubeMap;
uniform float level = 0.0f;

void main(){
    vec3 envColor = textureLod(cubeMap,WorldPos,level).rgb;

    // HDR and gamma correction
    envColor = envColor / (envColor + vec3(1.0f));
    envColor = pow(envColor, vec3(1.0f/2.2f));

    FragColor = vec4(envColor,1.0f);
}