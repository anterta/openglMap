#version 330 core

#ifdef VERTEX_SHADER

layout(location= 0) in vec3 position;
uniform mat4 mvpMatrix;

out vec2 textureCoords;

void main(void) {
	gl_Position = mvpMatrix * vec4(position,1);
	textureCoords = vec2(position.x/2.0 + 0.5, position.y/2.0 + 0.5); 
}
#endif

#ifdef FRAGMENT_SHADER

in vec2 textureCoords;

out vec4 fragment_color;

void main(void) {
	fragment_color = vec4(0.0, 0.0, 1.0, 1.0);
}
#endif