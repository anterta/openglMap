#version 330 core

#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;
layout(location= 3) in vec2 texcoord;
layout(location= 2) in vec3 normal;

layout(location= 1) in vec3 instance_position;

uniform mat4 mvpMatrix;

out vec4 vertex_position;

void main(){
    vertex_position= mvpMatrix * vec4(position + instance_position, 1);
    gl_Position= vertex_position;
}
#endif

#ifdef FRAGMENT_SHADER
in vec4 vertex_position;

out vec4 fragment_color;

void main(){
    vec3 color = (vertex_position.xyz/vertex_position.w)*0.5+0.5;
    fragment_color = vec4(color.z,color.z,color.z,1);
}
#endif