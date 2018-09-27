
#version 330

#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;
layout(location= 3) in vec2 texcoord;
layout(location= 2) in vec3 normal;

layout(location= 1) in vec3 instance_position;

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat4 normalMatrix;
uniform int nbCubes;

out vec3 vertex_position;
out vec3 vertex_normal;
out vec2 vertex_texcoord;

//out vec4 fragCol;
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


    p= position + p;
    gl_Position= mvpMatrix * vec4(p, 1);
    
    vertex_position= vec3(mvMatrix * vec4(p, 1));
    vertex_normal= mat3(normalMatrix) * normal;
    vertex_texcoord= texcoord;
}
#endif

#ifdef FRAGMENT_SHADER
in vec2 vertex_texcoord;
in vec3 vertex_position;
in vec3 vertex_normal;

//in vec4 fragCol;
flat in int choixTexture;

uniform sampler2D water_texture;
uniform sampler2D grass_texture;
uniform sampler2D ground_texture;
uniform sampler2D stone_texture;
uniform sampler2D snow_texture;

out vec4 fragment_color;

void main( )
{
    //float cos_theta= dot( normalize(vertex_normal), normalize(-vertex_position));
    //vec4 color;

    switch(choixTexture) {
        case 0:
            //color= texture(water_texture, vertex_texcoord);
            fragment_color= texture(water_texture, vertex_texcoord) * dot( normalize(vertex_normal), normalize(-vertex_position));
        break;
        case 1:
            //color= texture(grass_texture, vertex_texcoord);
            fragment_color= texture(grass_texture, vertex_texcoord) * dot( normalize(vertex_normal), normalize(-vertex_position));
        break;
        case 2:
            //color= texture(ground_texture, vertex_texcoord);
            fragment_color= texture(ground_texture, vertex_texcoord) * dot( normalize(vertex_normal), normalize(-vertex_position));
        break;
        case 3:
            //color= texture(stone_texture, vertex_texcoord);
            fragment_color= texture(stone_texture, vertex_texcoord) * dot( normalize(vertex_normal), normalize(-vertex_position));
        break;
        case 4:
            //color= texture(snow_texture, vertex_texcoord);
            fragment_color= texture(snow_texture, vertex_texcoord) * dot( normalize(vertex_normal), normalize(-vertex_position));
        break;
    }
    
    //fragment_color= cos_theta * color;
}

#endif
