
#ifndef _WATER_H
#define _WATER_H

#include "sun.h"

class Water
{
    public:
    Water() {}
    ~Water() {
        release_program(m_water_program);
        m_mesh.release();
        glDeleteTextures(1, &dudvMap);
        glDeleteTextures(1, &normalMap);
        glDeleteSamplers(1, &m_sampler);
    }

    bool initialiser(int nbRegions);
    Transform bindReflectionFrameBuffer(GLuint &program, Camera &cam, Sun &sun);

    Transform inverseCam(Camera &cam);

    void unbindReflectionFrameBuffer();

    void showReflection() {
        m_reflection.show();
    }

    void putTexture(GLuint &program);

    void move();

    void show(Transform view, Transform projection, Camera &cam, Terrain &terrain, Sun &sun);

    protected: 
    int REFLECTION_WIDTH = 640;
    int REFLECTION_HEIGHT = 360;

    float heightWater;
    float m_speed = 0.001;
    float m_move;
    float m_waveHeight = 0.02;
    float m_waveWidth = 1.5;

    Framebuffer m_reflection;
    GLuint dudvMap;
    GLuint normalMap;
    GLuint m_sampler;
	GLuint m_water_program;
    Mesh m_mesh;
};
#endif