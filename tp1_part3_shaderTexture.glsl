#version 330 core

#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;
layout(location= 3) in vec2 texcoord;
layout(location= 2) in vec3 normal;

layout(location= 1) in vec3 instance_position;

uniform mat4 mvpMatrix;
//uniform mat4 mvMatrix;
//uniform mat4 normalMatrix;

out vec4 vertex_position;
//out vec3 vertex_normal;
//out vec2 vertex_texcoord;

void main(){
    gl_Position= mvpMatrix * vec4(position + instance_position, 1);
    //vertex_texcoord = texcoord;
    vertex_position= mvpMatrix * vec4(position + instance_position, 1);
    //vertex_normal= mat3(normalMatrix) * normal;   // uniquement une rotation, mat3 suffit
}
#endif

#ifdef FRAGMENT_SHADER
in vec4 vertex_position;
//in vec3 vertex_normal;
//in vec2 vertex_texcoord;

layout(location = 1) out float Zbuffer;
out vec4 fragment_color;

void main(){
    Zbuffer = gl_FragCoord.z;
    vec3 color = (vertex_position.xyz/vertex_position.w)*0.5+0.5;
    fragment_color = vec4(color.z,color.z,color.z,1);
}
#endif