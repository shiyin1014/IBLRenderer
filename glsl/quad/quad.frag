#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D quadTexture;

void main(){

    vec3 color = texture(quadTexture,TexCoords).rgb;

    gl_FragColor = vec4(color,1.0f);
}