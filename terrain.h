
#ifndef _TERRAIN_H
#define _TERRAIN_H

#include "camera.h"
#include "billboards.h"


class Terrain
{
    public:
    Terrain() {}
    ~Terrain() {
        m_mesh.release();
    }
    
    void make_terrain( const char *filename, const int nbRegions, Billboards& billboards);

    int sizeRegion() { return m_sizeRegion; }
    Mesh mesh() { return m_mesh; }
    void bounds(Point &pmin, Point &pmax) { m_mesh.bounds(pmin,pmax); }
    int nbRegions() { return m_nbRegions; }
    vec3 get(int x, int y) { return m_mesh.positions()[indice(x,y)]; }

    float getHauteur(float x, float y);
    void hauteurCamera(Camera &cam);

    void bindBuffers( const Transform& vp );
    void multiDraw(const GLuint& program, const Transform& p, const Transform& v);
    void multiDraw();
    void release();

    protected:
    int indice(int x,int y) {
        return y/64 * m_nbRegions*4096 + y%64 * 64 + x/64 * 4096 + x%64;
    }

    Mesh m_mesh;
    Mesh m_cube;
    int m_nbRegions;
    int m_sizeRegion = 64*64;
    std::vector<vec2> m_min_max;
    std::vector<Transform> m_multi_model;
    std::vector<Object> m_objects;

    Multi_Draw m_multi_draw;
    
};

#endif
