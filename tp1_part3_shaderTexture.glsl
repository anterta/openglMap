#version 330 core

#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;
layout(location= 3) in vec2 texcoord;
layout(location= 2) in vec3 normal;

layout(location= 1) in vec3 instance_position;

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat4 normalMatrix;

out vec3 vertex_position;
out vec3 vertex_normal;

void main(){
    gl_Position= mvpMatrix * vec4(position + instance_position, 1);
    vertex_position= vec3(mvMatrix * vec4(position + instance_position, 1));
    vertex_normal= mat3(normalMatrix) * normal;   // uniquement une rotation, mat3 suffit 
    
}
#endif

#ifdef FRAGMENT_SHADER
in vec3 vertex_position;
in vec3 vertex_normal;

// Ouput data
layout(location = 1) out float fragmentdepth;
out vec4 fragment_color;

void main(){
    // place la source de lumiere sur la camera, repere camera, position 0, 0, 0
    float cos_theta= dot( normalize(vertex_normal), normalize(-vertex_position));

    fragment_color= cos_theta * vec4(0,1,0,1);

    // Not really needed, OpenGL does it anyway
    fragmentdepth = gl_FragCoord.z;
}
#endif