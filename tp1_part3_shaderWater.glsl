#version 430 core
#define M_PI 3.1415926535897932384626433832795

#ifdef VERTEX_SHADER

layout(location= 0) in vec3 position;
uniform mat4 mvpMatrix;
uniform float move;
uniform float waveHeight;
uniform float waveWidth;
uniform vec4 lightPos;

out vec4 pos;
out float moveF;
out float waveHeightF;
out vec2 textCoords;
out vec3 dirCamera;
out vec3 dirLight;
out vec3 lightColor;

void main(void) {
	moveF = move;
	waveHeightF = waveHeight;
	pos = mvpMatrix * vec4(position, 1.0);
	textCoords = position.xz * waveWidth;
	dirCamera = normalize( - pos.xyz);
	dirLight = normalize(pos.xyz - lightPos.xyz);

    float nuit = dot(vec3(0,1,0),dirLight);
    float r = 1 - nuit/2;
    float g = 0.5;
    float b = 4*nuit/10 + 0.1;
	lightColor = vec3(r,g,b);

	gl_Position = pos;
}
#endif

#ifdef FRAGMENT_SHADER
layout(early_fragment_tests) in;

in vec4 pos;
in float moveF;
in float waveHeightF;
in vec2 textCoords;
in vec3 dirCamera;
in vec3 dirLight;
in vec3 lightColor;


uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D dudvMap;
uniform sampler2D normalMap;

out vec4 fragment_color;

const float shineDamper = 2;
const float reflectivity = 10;

void main(void)
{
	vec2 refracCoords = (pos.xy/pos.w)/2+.5;
	
	float loop = textCoords.x + moveF;
	if(loop > .99)
		loop -= .98;

	vec2 distortion = texture(dudvMap, vec2(loop, textCoords.y)).rg * waveHeightF;

	loop = abs(-textCoords.x + moveF);
	if(loop > .99)
		loop -= .98;
	float loop2 = textCoords.y + moveF;
	if(loop2 > .99)
		loop2 -= .98;
	
	distortion += texture(dudvMap, vec2(loop, loop2)).rg * waveHeightF/2;
	
	refracCoords += distortion;
	refracCoords = clamp(refracCoords, 0.01, 0.99);
	
	vec2 reflecCoords = vec2(refracCoords.x,-refracCoords.y+1);
	
	vec4 reflectionColour = texture(reflectionTexture, reflecCoords);
	vec4 refractionColour = texture(refractionTexture, refracCoords); 

	vec4 normalMapCol = texture(normalMap, distortion);
	vec3 normal = normalize(vec3(normalMapCol.r * 2. - 1., normalMapCol.b, normalMapCol.g * 2. -1.));


    vec3 h = (dirLight+dirCamera)/2;
    float dist_sun = length(dirLight);
    float cos_theta= clamp(dot( normal, normalize(h)),0,1);
	float m = 16;
    float reflechissant = ((m+8)/8*M_PI)*pow(cos_theta,m);
	vec3 specularHighlights = lightColor * reflechissant/(dist_sun*dist_sun);/*
	
	vec3 reflectedLight = reflect(normalize(dirLight), normal);
	float specular = max(dot(reflectedLight, dirCamera), 0.0);
	specular = pow(specular, shineDamper);
	vec3 specularHighlights = lightColor * specular * reflectivity;*/

	float factor = clamp(.1 + 2*dot(dirCamera,vec3(0,-1,0)),0,1);
	fragment_color = mix(reflectionColour, 2*refractionColour, factor);
	fragment_color = mix(fragment_color, vec4(0, .3, .5, 1), .2);// + vec4(specularHighlights, 1);
}
#endif