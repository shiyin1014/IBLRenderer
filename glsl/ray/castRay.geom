#version 330 core

layout(lines) in;
layout(line_strip, max_vertices = 256) out;

const float PI = 3.14159265359;

float offsetPolarAngle = PI / 6.0f;
int numEmits = 127;

vec3 generatePointVec3(float phi){
    return vec3(sin(offsetPolarAngle)*sin(phi),sin(offsetPolarAngle)*cos(phi),cos(offsetPolarAngle));
}

void main(){

    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();
    EndPrimitive();

    float alphaLen = 0.1f;
    float totalLen = length(gl_in[0].gl_Position - gl_in[1].gl_Position);
    float len = totalLen * alphaLen;

    // 建立一个局部坐标系
    vec3 normal = normalize(gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz);
    vec3 N = normal;
    vec3 up = vec3(0,1,0);
    vec3 right = normalize(cross(up,N));
    up = normalize(cross(N,right));

    for(int i = 0;i < numEmits; i++){
        float phi = float(i) * (2.0f * PI) / float(numEmits);
        vec3 local_dir = generatePointVec3(phi);
        vec3 world_dir = local_dir.x * right + local_dir.y * up + local_dir.z * N;
        vec4 world_pos = vec4(gl_in[1].gl_Position.xyz + len * world_dir,1.0f);

        gl_Position = world_pos;
        EmitVertex();
        gl_Position = gl_in[1].gl_Position;
        EmitVertex();
        EndPrimitive();
    }
}