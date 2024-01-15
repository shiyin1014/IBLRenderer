#version 330 core

in vec2 uv;

uniform sampler2D depthMap;

void main(){
    float depth = texture(depthMap,uv).r;

    gl_FragColor = vec4(vec3(depth),1.0f);
}