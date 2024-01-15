#version 330 core

in vec2 TexCoords;

uniform sampler2D image;

uniform bool horizontal;
uniform float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main(){

    vec2 tex_offset = 1.0f / textureSize(image,0);
    vec3 result = texture(image,TexCoords).rgb * weight[0];

    if(horizontal){
        for(int i = 0;i < 5; i++){
            result += texture(image,TexCoords + vec2(i * tex_offset.x,0.0f)).rgb * weight[i];
            result += texture(image,TexCoords - vec2(i * tex_offset.x,0.0f)).rgb * weight[i];
        }
    }else{
        for(int i = 0;i < 5; i++){
            result += texture(image,TexCoords + vec2(i * tex_offset.y,0.0f)).rgb * weight[i];
            result += texture(image,TexCoords - vec2(i * tex_offset.y,0.0f)).rgb * weight[i];
        }
    }

    gl_FragColor = vec4(result,1.0f);
}