
#version 450

#ifdef VERTEX_SHADER

layout(location= 0) in vec3 position;

uniform mat4 mvpMatrix;

void main( )
{
    gl_Position= mvpMatrix * vec4(position, 1);
}

#endif

#ifdef FRAGMENT_SHADER

#extension GL_ARB_shader_ballot : require
#extension GL_AMD_gpu_shader_int64 : enable
#extension GL_ARB_gpu_shader_int64 : enable

out vec4 fragment_color;


// bizarre, mais bitCount( uint64_t ) n'existe pas... 
uint bits( const in uint64_t mask )
{
    // decoupe le uint64 en 2 uint32
    uvec2 vmask= unpackUint2x32(mask);
    // renvoie le nombre de bits a 1
    return bitCount(vmask.y) + bitCount(vmask.x);
}


void main( )
{
    // recupere les shaders / threads pour lesquels la condition est vraie
    // gl_HelperInvocation == true si le fragment est a l'exterieur du triangle
    uint64_t mask= ballotARB(gl_HelperInvocation == false);
    
    // compte le nombre de threads / shaders
    uint threads= bits(mask);
    uint helpers= gl_SubGroupSizeARB - threads;
    // gl_SubGroupSize indique le nombre de shaders / threads ordonnances sur la meme unite de calcul
    // 64 pour amd, et 32 pour nvidia, variable pour intel
    
    // construit une couleur
    float v= float(threads) / float(gl_SubGroupSizeARB);
    fragment_color= vec4(1.0 - v, v, 0, 1);
    
    // affiche en vert la proportion de shaders / threads actifs (les fragments sont a l'interieur du triangle)
    // et affiche en rouge la proportion de shaders / threads inactifs (les fragments sont a l'exterieur du triangle) 
}

#endif
