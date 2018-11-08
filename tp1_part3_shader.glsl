
#version 330
#define M_PI 3.1415926535897932384626433832795

float random2d (vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;
layout(location= 3) in vec2 texcoord;
layout(location= 2) in vec3 normal;

layout(location= 1) in vec3 instance_position;

uniform mat4 mvpMatrix;
uniform mat4 vMatrix;
uniform int nbCubes;

uniform mat4 sunMvpMatrix;
uniform mat4 sunInverseMatrix;

out vec4 vertex_position;
out vec4 vertex_position_sun;
out vec3 vertex_normal;
out vec2 vertex_texcoord;
out vec3 sun_direction;
out vec3 obs_direction;

out vec3 cube_pos;

flat out int choixTexture;

void main( )
{
    vec3 p = instance_position;
    float y = p.y - random2d(p.xz);
    float s = float(nbCubes)/15.0;
    vertex_normal= (vMatrix * vec4(normal,0)).xyz;

    if(y < 0.05*s) {
        choixTexture = 0;
        p.y = 0.05*s +  (sin(p.x*5.0 + p.z*1.0)+random2d(p.xz))/2;
        vertex_normal= (vMatrix * vec4(0,1,0,0)).xyz; // todo mouvement en fonction du temps
    } else if(y < 0.3*s)
        choixTexture = 1;
    else if(y < 0.5*s)
        choixTexture = 2;
    else if(y < 0.7*s)
        choixTexture = 3;
    else
        choixTexture = 4;

    p= position + p;
    gl_Position= mvpMatrix * vec4(p, 1);
    
    vertex_position= mvpMatrix * vec4(p, 1);
    vertex_position_sun= sunMvpMatrix * vec4(p, 1);
    vertex_texcoord= texcoord;
    vec3 position_sun = (vMatrix * (sunInverseMatrix) * vec4(0,0,0,1)).xyz;
    obs_direction = vec3(0,0,0) - (vMatrix * vec4(p, 1)).xyz;
    sun_direction = position_sun + obs_direction;
    cube_pos = p;
}
#endif

#ifdef FRAGMENT_SHADER
uniform sampler2D zBuffer_texture;

in vec2 vertex_texcoord;
in vec4 vertex_position;
in vec4 vertex_position_sun;
in vec3 vertex_normal;
in vec3 sun_direction;
in vec3 obs_direction;

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
    // La texture en fonction de la hauteur
    vec4 color;

    switch(choixTexture) {
        case 0:
            color= texture(water_texture, vertex_texcoord);
        break;
        case 1:
            color= texture(grass_side_texture, vertex_texcoord);
            if( cube_pos.y > 0.49)
                color= texture(grass_top_texture, vertex_texcoord);
        break;
        case 2:
            color= texture(ground_texture, vertex_texcoord);
        break;
        case 3:
            color= texture(stone_texture, vertex_texcoord);
        break;
        case 4:
            color= texture(snow_texture, vertex_texcoord);
        break;
    }

    vec3 ShadowCoord = vertex_position_sun.xyz/vertex_position_sun.w;
    ShadowCoord = ShadowCoord * 0.5 + 0.5;
    vec4 shadowColor = texture( zBuffer_texture, ShadowCoord.xy);
    
    float visibility = 1.0;
    if ( shadowColor.r  < ShadowCoord.z - 0.01)
        visibility = 0.4;


    float m = 16;
    float k = 0.8;
    float intensity = 1000000;
    vec3 h = (sun_direction+obs_direction)/2;
    float dist_sun = length(sun_direction);
    float cos_theta= clamp(dot( vertex_normal, normalize(h)),0,1);
    float diffus = 1/M_PI;
    float reflechissant = ((m+8)/8*M_PI)*pow(cos_theta,m);
    float mixte = intensity * (k*diffus + (1-k)*reflechissant)/(dist_sun*dist_sun);

    float nuit = dot(vec3(0,1,0),normalize(sun_direction));
    nuit += (fract(sin(nuit)*100000.0))/8;
    if(nuit < 0.25)
        visibility = max(0., nuit*4+0.1);

    
    float r = 1 - nuit/2;
    float g = 0.5;
    float b = 4*nuit/10 + 0.1;

    fragment_color= color * mixte * visibility * 0.8
                    + vec4(r,g,b,1) * mixte * visibility * 0.2;
}

#endif
