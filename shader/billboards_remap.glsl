#version 430

#ifdef VERTEX_SHADER
#extension GL_ARB_shader_draw_parameters : require

layout(location= 0) in vec3 position;
layout(location= 1) in vec2 texcoord;
uniform mat4 vpMatrix;
uniform mat4 inverseMatrix;
uniform vec3 skyColor;

out vec2 textCoords;
out float fog;
out vec3 skyColorF;


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

    // Calcul pour le brouillard
    vec3 position_obs   = (inverseMatrix    * vec4(0,0,0,1)).xyz;
    vec3 obs_direction  = position_obs - p.xyz;
    fog = exp(-pow(length(obs_direction)*0.01,5.));
    fog = clamp(fog,0.0,1.0);
    skyColorF = skyColor;

	gl_Position = vpMatrix * p;
}
#endif

#ifdef FRAGMENT_SHADER

in vec2 textCoords;
in float fog;
in vec3 skyColorF;
uniform sampler2D texture0;

out vec4 fragment_color;

void main(void)
{
    vec4 color = texture(texture0, textCoords);
    if(color.a < .9)
        discard;
    fragment_color = vec4(mix(skyColorF,color.xyz,fog),1);
}
#endif