//! \file tuto9_color.glsl

#version 330

#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;

uniform mat4 mvpMatrix;

out vec4 fragCol;

void main( )
{
    gl_Position= mvpMatrix * vec4(position, 1);
    float r = fract(sin(position.y)*1000);
    float y = (position.y + r/5)/1.2;

    if(y < 0.5)
        fragCol= vec4(0.f,0.f,1.f,0.1f);
    else if(y < 4.f)
        fragCol= vec4(0.3f,1.f,0.3f,1.f);
    else if(y < 10.f)
        fragCol= vec4(0.7f,0.7f,0.7f,1.f);
    else
        fragCol= vec4(1.f,1.f,1.f,1.f);
}

#endif


#ifdef FRAGMENT_SHADER
in vec4 fragCol;
out vec4 fragment_color;

uniform vec4 color;

void main( )
{
    fragment_color = fragCol;
}

#endif
