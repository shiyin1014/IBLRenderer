#version 330 core

in vec2 TexCoords;

uniform sampler2D color;        // colorImage

uniform int process;            // choice of post process

// gaussian parameters
uniform int gaussian_size;      // kernel size
uniform float gaussian_sigma;   // sigma
#define maxSize 21              // max size
float kernel[maxSize];

// sharpen parameters
uniform int sharpen_kernel;


// gaussian blur --------------------------------------------------------------
// from https://www.shadertoy.com/view/XdfGDH
float normpdf(in float x, in float sigma)
{
    return 0.39894*exp(-0.5*x*x/(sigma*sigma))/sigma;
}
vec3 GaussianBlur(){
    vec3 res = vec3(0.0f);
    int kSize = (gaussian_size - 1) / 2;

    // create the 1-D kernel
    float Z = 0.0f;
    for(int j = 0;j <= kSize; ++j){
        kernel[kSize + j] = kernel[kSize - j] = normpdf(float(j), gaussian_sigma);
    }

    //get the normalization factor (as the gaussian has been clamped)
    for (int j = 0; j < gaussian_size; ++j)
    {
        Z += kernel[j];
    }

    vec2 texelSize = 1.0f / textureSize(color,0);

    // read out the texels
    for (int i=-kSize; i <= kSize; ++i)
    {
        for (int j=-kSize; j <= kSize; ++j)
        {
            res += kernel[kSize+j] * kernel[kSize+i] * texture(color, TexCoords + vec2(i,j) * texelSize ).rgb;
        }
    }

    return res / (Z*Z);
}
// gaussian blur --------------------------------------------------------------



// sharpen --------------------------------------------------------------------
// https://www.shadertoy.com/view/XlycDV
vec3 Sharpen(){
    vec3 res = vec3(0.0f);

    vec2 texelSize = 1.0f / textureSize(color,0);
    // current
    vec3 currentColor = texture(color,TexCoords).rgb;
    // others, 3*3 kernel size
    vec3 top = texture(color,TexCoords + vec2(0,1) * texelSize).rgb;
    vec3 left = texture(color,TexCoords + vec2(-1,0) * texelSize).rgb;
    vec3 right = texture(color,TexCoords + vec2(1,0) * texelSize).rgb;
    vec3 bottom = texture(color,TexCoords + vec2(0,-1) * texelSize).rgb;

    if(sharpen_kernel == 5){
        res += 5.0f * currentColor - top - left - right - bottom;
        return res;
    }

    vec3 rightTop = vec3(0.0f);
    vec3 leftTop = vec3(0.0f);
    vec3 leftBottom = vec3(0.0f);
    vec3 rightBottom = vec3(0.0f);

    if(sharpen_kernel == 9){
        leftTop = texture(color,TexCoords + vec2(-1,1) * texelSize).rgb;
        rightTop = texture(color,TexCoords + vec2(1,1) * texelSize).rgb;
        leftBottom = texture(color,TexCoords + vec2(-1,-1) * texelSize).rgb;
        rightBottom = texture(color,TexCoords + vec2(1,-1) * texelSize).rgb;
    }

    res += 9.0f * currentColor - top - left - right - bottom - leftTop - leftBottom - rightTop - rightBottom;

    return res;
}
// sharpen --------------------------------------------------------------------


void main(){
    vec3 finalColor = vec3(0.0f);

    if(process == 0){
        finalColor = texture(color,TexCoords).rgb;
    }else if(process == 1){
        finalColor = GaussianBlur();
    }else if(process == 2){
        finalColor = Sharpen();
    }

    gl_FragColor = vec4(finalColor,1.0f);
}