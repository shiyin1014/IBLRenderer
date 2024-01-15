#version 330 core

in vec2 TexCoords;

uniform bool bloom = false;

uniform sampler2D sceneColor;       // scene color
uniform sampler2D blurColor;        // blur outline

void main(){

    vec3 scene = texture(sceneColor,TexCoords).rgb;

    if(bloom){
        scene += texture(blurColor,TexCoords).rgb;
    }

    gl_FragColor = vec4(scene,1.0f);
}