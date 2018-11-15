
#version 330
#define M_PI 3.1415926535897932384626433832795

float random2d (vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;
layout(location= 1) in vec2 texcoord;
layout(location= 2) in vec3 instance_position;
layout(location= 3) in vec3 normal;


uniform mat4 mvpMatrix;
uniform mat4 vMatrix;
uniform int nbCubes;

uniform int inverse;

uniform mat4 sunMvpMatrix;
uniform mat4 sunInverseMatrix;
uniform mat4 inverseMatrix;

uniform int clip;

out vec4 vertex_position;
out vec4 vertex_position_sun;
out vec3 vertex_normal;
out vec2 vertex_texcoord;
out vec3 sun_direction;
out vec3 obs_direction;

out vec3 cube_pos;
flat out int tailleTerrain;

flat out int choixTexture;


void main( )
{
    tailleTerrain = nbCubes;
    gl_ClipDistance[0] = clip;
    
    vec3 p = instance_position;
    float y = p.y - random2d(p.xz);
    vertex_normal= normal;//(vMatrix * vec4(normal,0)).xyz;

    if(y < 0.0033*nbCubes) {
    gl_ClipDistance[0] = -clip;
        choixTexture = 1;
        //p.y = p.y-.5;
    } else if(y < 0.02*nbCubes)
        choixTexture = 1;
    else if(y < 0.033*nbCubes)
        choixTexture = 2;
    else if(y < 0.0467*nbCubes)
        choixTexture = 3;
    else
        choixTexture = 4;

    p= position + p;
    vertex_position= mvpMatrix * vec4(p, 1);
    if(inverse == 1)
        vertex_position.z *= -1;

    gl_Position= vertex_position;
    
    vertex_position_sun= sunMvpMatrix * vec4(p, 1);
    vertex_texcoord= texcoord;
    vec3 position_sun = (sunInverseMatrix * vec4(0,0,0,1)).xyz;
    vec3 position_obs = (inverseMatrix * vec4(0,0,0,1)).xyz;
    obs_direction = position_obs - p;
    sun_direction = position_sun - p;//position_sun + obs_direction;
    cube_pos = position;
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
flat in int tailleTerrain;

flat in int choixTexture;

//uniform sampler2D water_texture;
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
            color= texture(grass_top_texture, vertex_texcoord);
            color.a = 0;
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
    if ( shadowColor.r  < ShadowCoord.z - 0.02)
        visibility = 0.4;

    float m = 2;
    float k = 0.9;
    float intensity = 3*tailleTerrain*tailleTerrain;
    vec3 h = (sun_direction+obs_direction)/2;
    float dist_sun = length(sun_direction);
    float cos_theta= clamp(dot( vertex_normal, normalize(h)),0,1);

    float diffus = 1/M_PI;
    float reflechissant = ((m+8)/8*M_PI)*pow(cos_theta,m);
    float mixte = intensity * (k*diffus + (1-k)*reflechissant)/(dist_sun*dist_sun);

    float nuit = dot(vec3(0,1,0),normalize(sun_direction));
    nuit += (fract(sin(nuit)*100000.0))/8;
    if(nuit < 0.25)
        visibility = max(0., min(visibility, nuit*4+0.1));

    
    float r = 1 - nuit/2;
    float g = 0.5;
    float b = 4*nuit/10 + 0.1;

    vec3 col =  color.xyz * mixte * visibility * 0.8
                    + vec3(r,g,b) * mixte * visibility * 0.2;
    fragment_color= vec4(col,color.a);
}

#endif
