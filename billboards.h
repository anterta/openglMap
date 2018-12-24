
#ifndef _BILLBOARDS_H
#define _BILLBOARDS_H

#include "multiDraw.h"

#include "gKit/texture.h"
#include "gKit/mesh.h"


class Billboards
{
    public:
    Billboards() {}
    ~Billboards() {
		release_program(m_program);
    }

    void initialiser();
    void createBuffers();

    void addPos(Vector p) {
        m_multi_draw.addPos(p, 5.f);
    }
    
    void show(Transform& p, Transform& v, vec3 skyColor);    
    void showForShadow(Transform& p, Transform& v, GLuint program);

    private:
    Mesh m_mesh;
    GLuint m_texture;
    GLuint m_program;
    Multi_Draw m_multi_draw;

};
#endif