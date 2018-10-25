
#version 330
#define M_PI 3.1415926535897932384626433832795

#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;
layout(location= 3) in vec2 texcoord;
layout(location= 2) in vec3 normal;

layout(location= 1) in vec3 instance_position;

uniform mat4 mvpMatrix;
uniform mat4 normalMatrix;
uniform int nbCubes;

uniform mat4 sunMvpMatrix;
uniform mat4 sunMvMatrix;
uniform mat4 sunNormalMatrix;

out vec4 vertex_position;
out vec4 vertex_position_sun;
out vec3 vertex_normal;
out vec2 vertex_texcoord;
out vec3 position_sun;
out vec3 position_obs;

out vec3 cube_pos;

flat out int choixTexture;

void main( )
{
    vec3 p = instance_position;
    float y = p.y - fract(sin(p.y)*1000);
    float s = float(nbCubes)/15.0;

    if(y < 0.05*s) {
        choixTexture = 0; // fragCol= vec4(0.f,0.f,1.f,0.1f);
        p.y = 0.05*s;
    } else if(y < 0.3*s)
        choixTexture = 1; // fragCol= vec4(0.3f,1.f,0.3f,1.f);
    else if(y < 0.5*s)
        choixTexture = 2; // fragCol= vec4(0.7f,0.7f,0.7f,1.f);
    else if(y < 0.7*s)
        choixTexture = 3; // fragCol= vec4(0.7f,0.7f,0.7f,1.f);
    else
        choixTexture = 4; // fragCol= vec4(1.f,1.f,1.f,1.f);

    //p.y = Zbuffer;

    p= position + p;
    gl_Position= mvpMatrix * vec4(p, 1);
    
    vertex_position= mvpMatrix * vec4(p, 1);
    vertex_position_sun= sunMvpMatrix * vec4(p, 1);
    vertex_normal= mat3(sunNormalMatrix) * normal;
    vertex_texcoord= texcoord;
    position_sun = mat3(sunMvpMatrix) * vec3(0,0,0);
    position_obs = mat3(mvpMatrix) * vec3(0,0,0);
    cube_pos = position;
}
#endif

#ifdef FRAGMENT_SHADER
uniform sampler2D zBuffer_texture;

in vec2 vertex_texcoord;
in vec4 vertex_position;
in vec4 vertex_position_sun;
in vec3 vertex_normal;
in vec3 position_sun;
in vec3 position_obs;

in vec3 cube_pos;

flat in int choixTexture;

uniform sampler2D water_texture;
uniform sampler2D grass_top_texture;
uniform sampler2D grass_side_texture;
uniform sampler2D ground_texture;
uniform sampler2D stone_texture;
uniform sampler2D snow_texture;

out vec4 fragment_color;

void main( )
{
    vec3 ShadowCoord = vertex_position_sun.xyz/vertex_position_sun.w;
    ShadowCoord = ShadowCoord * 0.5 + 0.5;
    vec4 shadowColor = texture( zBuffer_texture, ShadowCoord.xy);
    
    float visibility = 1.0;
    if ( shadowColor.r  < ShadowCoord.z - 0.01)
        visibility = 0.4;


	vec3 lightDirection = vertex_position_sun.xyz/vertex_position_sun.w-position_sun;
	vec3 observerDirection = vertex_position.xyz/vertex_position.w-position_obs;

    float m = 1;
    float k = 0.8;
    vec3 h = (lightDirection+observerDirection)/2;
    float cos_theta= max(0,dot( vertex_normal, normalize(h)));
    float diffus = 1/M_PI;
    float reflechissant = ((m+8)/8*M_PI)*pow(cos_theta,m);
    float mixte = k*diffus + (1-k)*reflechissant;

    if(reflechissant == 0)
        visibility = 0;

    vec4 color;

    switch(choixTexture) {
        case 0:
            color= texture(water_texture, vertex_texcoord);
            //fragment_color= texture(water_texture, vertex_texcoord) * dot( normalize(vertex_normal), normalize(-vertex_position) * visibility);
        break;
        case 1:
            color= texture(grass_side_texture, vertex_texcoord);
            if( cube_pos.y > 0.49)
                color= texture(grass_top_texture, vertex_texcoord);
            //fragment_color= texture(grass_texture, vertex_texcoord) * dot( normalize(vertex_normal), normalize(-vertex_position) * visibility);
        break;
        case 2:
            color= texture(ground_texture, vertex_texcoord);
            //fragment_color= texture(ground_texture, vertex_texcoord) * dot( normalize(vertex_normal), normalize(-vertex_position) * visibility);
        break;
        case 3:
            color= texture(stone_texture, vertex_texcoord);
            //fragment_color= texture(stone_texture, vertex_texcoord) * dot( normalize(vertex_normal), normalize(-vertex_position) * visibility);
        break;
        case 4:
            color= texture(snow_texture, vertex_texcoord);
            //fragment_color= texture(snow_texture, vertex_texcoord) * dot( normalize(vertex_normal), normalize(-vertex_position) * visibility);
        break;
    }
    
    fragment_color= color * mixte * visibility;
}

#endif
