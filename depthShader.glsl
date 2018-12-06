#version 430 core

#ifdef VERTEX_SHADER

#extension GL_ARB_shader_draw_parameters : require

layout(location= 0) in vec3 position;

uniform mat4 vpMatrix;

out vec4 vertex_position;

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
    vertex_position= vpMatrix  * objectMatrix[id] * vec4(position, 1);
    gl_Position= vertex_position;
}
#endif

#ifdef FRAGMENT_SHADER
layout(early_fragment_tests) in;
in vec4 vertex_position;

out vec4 fragment_color;
//layout(location = 1) out float Zbuffer;

void main(){
    //Zbuffer = gl_FragCoord.z;
    vec3 color = (vertex_position.xyz/vertex_position.w)*0.5+0.5;
    fragment_color = vec4(color.z,color.z,color.z,1);
}
#endif