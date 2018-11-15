
#ifndef _WATER_H
#define _WATER_H

#include "framebuffer.h"
#include "camera.h"
#include "texture.h"
#include "terrain.h"

class Water
{
    public:
    Water() {};

    void initialiser(int nbRegions) {
        heightWater = nbRegions*0.2113;
        m_reflection.initialiseFrameBuffer(REFLECTION_WIDTH,REFLECTION_HEIGHT);
        m_refraction.initialiseFrameBuffer(REFRACTION_WIDTH,REFRACTION_HEIGHT);
    
        m_mesh = read_mesh("data/carre.obj");

        m_water_program= read_program("src/tp1_part3_shaderWater.glsl");
        program_print_errors(m_water_program);

        dudvMap = read_texture(0, "data/terrain/waterDUDV.png");
        normalMap = read_texture(0, "data/terrain/waterNormalMap.png");
        m_move = 0.;
        
        // cree le sampler
        glGenSamplers(1, &sampler);
        glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    }

    void bindRefractionFrameBuffer(GLuint &program) {
        m_refraction.bindFrameBuffer();
        glEnable(GL_CLIP_DISTANCE0);
        program_uniform(program, "clip", -1);
    }

    void unbindRefractionFrameBuffer() {
        m_refraction.unbindCurrentFrameBuffer();
        glDisable(GL_CLIP_DISTANCE0);
    }

    Transform bindReflectionFrameBuffer(GLuint &program, Camera &cam) {
        m_reflection.bindFrameBuffer();
        glEnable(GL_CLIP_DISTANCE0);
        program_uniform(program, "clip", 1);
        Point p = cam.position();
        return cam.lookAt(Vector(0,2*(p.y-heightWater),0),true);
    }

    void unbindReflectionFrameBuffer() {
        m_reflection.unbindCurrentFrameBuffer();
        glDisable(GL_CLIP_DISTANCE0);
    }

    void showReflection() {
        m_reflection.show();
    }

    void showRefraction() {
        m_refraction.show();
    }

    void release() {
        m_reflection.release();
        m_refraction.release();
    }

    void putTexture(GLuint &program) {
        program_uniform(program, "move", m_move);
        program_uniform(program, "waveHeight", m_waveHeight);
        program_uniform(program, "waveWidth", m_waveWidth);

        GLint location;
        location= glGetUniformLocation(program, "reflectionTexture");
        glUniform1i(location, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,  m_reflection.getColorTexture());
        glBindSampler(0, sampler);

        location= glGetUniformLocation(program, "refractionTexture");
        glUniform1i(location, 1);
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, m_refraction.getColorTexture());
        glBindSampler(1, sampler);

        location= glGetUniformLocation(program, "dudvMap");
        glUniform1i(location, 2);
        glActiveTexture(GL_TEXTURE0 +2);
        glBindTexture(GL_TEXTURE_2D, dudvMap);
        glBindSampler(2, sampler);

        location= glGetUniformLocation(program, "normalMap");
        glUniform1i(location, 3);
        glActiveTexture(GL_TEXTURE0 +3);
        glBindTexture(GL_TEXTURE_2D, normalMap);
        glBindSampler(3, sampler);
    }

    void move() {
            m_move += m_speed;
        if(m_move>1)
            m_move -= 1;
    }

    void show(Transform view, Transform projection, Camera cam, Terrain terrain, Sun sun, std::vector<int> regions) {
        int nbRegions = terrain.nbRegions();
        glUseProgram(m_water_program);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        putTexture(m_water_program);
        vec4 position_sun = projection*view*(sun.view().inverse()( vec4(0,0,0,1)));
        program_uniform(m_water_program, "lightPos", position_sun);

        
        vec3 p = terrain.get(nbRegions*32,nbRegions*32);
        Transform t = Translation(Vector(1,1+nbRegions*.2112,1)) * Scale(nbRegions*64,1,-64*nbRegions);
                program_uniform(m_water_program, "mvpMatrix", projection*view*t);
                draw(m_mesh,m_water_program);/*
        //printf("test %d\n", regions.size());
        for(int i=0; i<regions.size(); i++)
            if(m_min_max[regions[i]].x <= nbRegions*0.2113) {
                vec3 p = m_mesh.positions()[regions[i]*m_sizeRegion];
                Transform t = Translation(Vector(p.x,1+nbRegions*.2112,p.z)) * Scale(64,1,-64);
                program_uniform(m_water_program, "mvpMatrix", projection*view*t);
                draw(m_water,m_water_program);
            }*/
    }

    protected: 
    int REFLECTION_WIDTH = 640;//320;
    int REFLECTION_HEIGHT = 360;//180;
    int REFRACTION_WIDTH = 1280;
    int REFRACTION_HEIGHT = 720;

    float heightWater;
    float m_speed = 0.001;
    float m_move;
    float m_waveHeight = 0.02;
    float m_waveWidth = 1.5;

    Framebuffer m_reflection;
    Framebuffer m_refraction;
    GLuint dudvMap;
    GLuint normalMap;
    GLuint sampler;
	GLuint m_water_program;
    Mesh m_mesh;
};
#endif