#version 430

#ifdef VERTEX_SHADER
#extension GL_ARB_shader_draw_parameters : require

layout(location= 0) in vec3 position;
layout(location= 1) in vec2 texcoord;
uniform mat4 mvpMatrix;

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


void main(void) {
    textCoords = texcoord;
    uint id= remap[gl_DrawIDARB];
    vec4 p = objectMatrix[id] * vec4(position, 1);
	gl_Position = mvpMatrix * p;
}
#endif

#ifdef FRAGMENT_SHADER

in vec2 textCoords;
uniform sampler2D texture0;

out vec4 fragment_color;

void main(void)
{
    vec4 color = texture(texture0, textCoords);/*
    if(color.a < .5)
        discard;*/
	fragment_color = vec4(1,0,0,1);//color;
}
#endif