#version 330 core

layout(location = 0) out vec4 sceneColor;       // 场景的边框绘制成全红色
layout(location = 1) out vec4 outlineColor;     // 将边框颜色单独输出到第二个纹理附件中用作模糊处理

out vec4 FragColor;

void main(){
    sceneColor = vec4(1.0f,0.0f,0.0f,1.0f);
    outlineColor = sceneColor;
}