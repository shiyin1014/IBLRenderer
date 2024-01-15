#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 WorldPos;

void main(){

    WorldPos = aPos;

    mat4 RotView = mat4(mat3(view));
    vec4 clipPos = projection * RotView * vec4(aPos,1.0f);

    gl_Position = clipPos.xyww;
}