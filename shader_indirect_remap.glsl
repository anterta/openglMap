
#version 430
#define M_PI 3.1415926535897932384626433832795

float random2d (vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

#ifdef VERTEX_SHADER

#extension GL_ARB_shader_draw_parameters : require

layout(location= 0) in vec3 position;
layout(location= 1) in vec2 texcoord;
layout(location= 2) in vec3 instance_position;
layout(location= 3) in vec3 normal;

uniform mat4 vpMatrix;

uniform mat4 sunMvpMatrix;
uniform mat4 sunInverseMatrix;
uniform mat4 inverseMatrix;

uniform int nbCubes;
uniform int inverse;
uniform int clip;
uniform vec3 skyColor;

out vec3 cube_pos;
flat out int choixTexture;

out vec3 vertex_position;
out vec4 vertex_position_sun;
out vec2 vertex_texcoord;
out vec3 h;
out float fog;
out float nuit;
out vec3 skyColorF;
out float tailleTextShadow;

// row_major : organisation des matrices par lignes...
layout(binding= 0, row_major, std430) readonly buffer modelData
{
    mat4 objectMatrix[];
};

layout(binding= 1, std430) readonly buffer remapData
{
    uint remap[];
};

void main( )
{
    //gl_ClipDistance[0] = clip;
    
    uint id= remap[gl_DrawIDARB];
    vec4 p = objectMatrix[id] * vec4(position, 1);
    vec4 tmp = objectMatrix[id] * vec4(0,0,0,1);
    
    float y = tmp.y - random2d(tmp.xz);
    
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

    if(inverse == 1)
        p.z *= -1;
        
    // position dans le repere camera
    vertex_position= vec3(vpMatrix * p);
    gl_Position = vpMatrix * p;

    vertex_position_sun = sunMvpMatrix * p;
    vertex_texcoord     = texcoord;
    vec3 position_sun   = (sunInverseMatrix * vec4(0,0,0,1)).xyz;
    vec3 position_obs   = (inverseMatrix    * vec4(0,0,0,1)).xyz;
    vec3 obs_direction  = position_obs - p.xyz;
    vec3 sun_direction  = normalize(position_sun - p.xyz);
    h                   = (sun_direction + normalize(obs_direction))/2;
    cube_pos            = position;
    fog = exp(-pow(length(obs_direction)*0.01,5.));
    fog = clamp(fog,0.0,1.0);
    nuit = dot(vec3(0,1,0),sun_direction);
    skyColorF = skyColor;
    tailleTextShadow = 1.0/4096.0;
}
#endif


#ifdef FRAGMENT_SHADER
layout(early_fragment_tests) in;
uniform sampler2D zBuffer_texture;

in vec2 vertex_texcoord;
in vec3 vertex_position;
in vec4 vertex_position_sun;
in vec3 h;
in float fog;
in float nuit;
in vec3 skyColorF;

in vec3 cube_pos;
in float tailleTextShadow;

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

    // la shadow map
    vec3 ShadowCoord = vertex_position_sun.xyz/vertex_position_sun.w;
    ShadowCoord = ShadowCoord * 0.5 + 0.5;
    
    float total = 0.0;
    for(int x=-1; x <= 1; x++)
        for(int y=-1; y<= 1; y++) {
            float shadowColor = texture( zBuffer_texture, ShadowCoord.xy + vec2(x,y) * tailleTextShadow).r;
            if ( shadowColor  < ShadowCoord.z - 0.01)
                total += 1.0;
        }

    float visibility = 1.0 - total/15.0;

    // la lumière
    vec3 tn= normalize(dFdx(vertex_position));
    vec3 bn= normalize(dFdy(vertex_position));
    vec3 vertex_normal = normalize(cross(tn, bn));

    float m = 2;
    float k = 0.9;
    float cos_theta= clamp(dot( vertex_normal, h),0,1);

    float diffus = 1/M_PI;
    float reflechissant = ((m+8)/8*M_PI)*pow(cos_theta,m);
    float mixte = 2 * ( k*diffus + (1-k)*reflechissant );

    // coucher et levee de soleil
    float nuitBruit = nuit + (fract(sin(nuit)*100000.0))/8;
    if(nuitBruit < 0.25)
        visibility = max(0., min(visibility, nuitBruit*4+0.1));

    // Sortie
    //fragment_color= vec4(color.xyz * mixte * visibility,1);
    fragment_color = vec4(mix(skyColorF,color.xyz * mixte * visibility,fog),1);
}

#endif
