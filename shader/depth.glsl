#version 430 core

#ifdef VERTEX_SHADER

#extension GL_ARB_shader_draw_parameters : require

layout(location= 0) in vec3 position;
layout(location= 1) in vec2 texcoord;

uniform mat4 vpMatrix;

out vec2 textCoords;

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
    textCoords = texcoord;
    uint id= remap[gl_DrawIDARB];
    gl_Position= vpMatrix * objectMatrix[id] * vec4(position, 1);
}
#endif

#ifdef FRAGMENT_SHADER

in vec2 textCoords;
uniform sampler2D texture0;
out vec4 fragment_color;

void main(){
    vec4 color = texture(texture0, textCoords);
    if(color.a < .9)
        discard;
}
#endif