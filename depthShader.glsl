#version 430 core

#ifdef VERTEX_SHADER

#extension GL_ARB_shader_draw_parameters : require

layout(location= 0) in vec3 position;

uniform mat4 vpMatrix;

// row_major : organisation des matrices par lignes...
layout(binding= 0, row_major, std430) readonly buffer modelData
{
    mat4 objectMatrix[];
};

layout(binding= 1, std430) readonly buffer remapData
{
    uint remap[];
};

void main(){
    uint id= remap[gl_DrawIDARB];
    gl_Position= vpMatrix  * objectMatrix[id] * vec4(position, 1);;
}
#endif

#ifdef FRAGMENT_SHADER

void main(){
}
#endif