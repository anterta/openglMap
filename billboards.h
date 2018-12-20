
#ifndef _BILLBOARDS_H
#define _BILLBOARDS_H

#include "texture.h"
#include "camera.h"
#include "multiDraw.h"



class Billboards
{
    public:
    Billboards() {                    
        m_mesh = read_mesh("data/billboard/billboard.obj");
        m_texture = read_texture(0, "data/billboard/arbre.png");
        m_program = read_program("src/billboards_shader.glsl");
        program_print_errors(m_program);
        m_multi_draw.initialiser(m_mesh);
        m_multi_draw.createBuffer("src/shader_indirect_cull.glsl"); 
    }

    void addPos(Vector p) {
        m_multi_draw.addPos(p);
    }
    
    void show(Transform mvp) {
        m_multi_draw.bindBuffers(mvp);/*
        glDisable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/

        glUseProgram(m_program);
        program_use_texture(m_program, "texture0", 0, m_texture);
        m_multi_draw.multiDraw();
    }

    private:
    Mesh m_mesh;
    GLuint m_texture;
    GLuint m_program;
    Multi_Draw m_multi_draw;

};
#endif