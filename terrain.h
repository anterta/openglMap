
#ifndef _TERRAIN_H
#define _TERRAIN_H

#include "mat.h"
#include "mesh.h"
#include "image.h"
#include "image_io.h"

class Terrain
{
    public:
    Terrain() {};
    
    void make_terrain( const char *filename, const int nbRegions );
    void createBuffer(GLuint &buffer);
    bool visbleCamera(int i, Transform mvp);

    void drawRegion(int i, int vertex_count) {
        glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, vertex_count, m_sizeRegion, i*m_sizeRegion);
    }

    int sizeRegion() { return m_sizeRegion; }
    Mesh mesh() { return m_mesh; }
    void bounds(Point &pmin, Point &pmax) { m_mesh.bounds(pmin,pmax); }
    int nbRegions() { return m_nbRegions; }
    vec3 get(int x, int y) { return m_mesh.positions()[indice(x,y)]; }
    float getHauteur(float x, float y);

    protected:
    int indice(int x,int y) {
        //int r = y * m_nbRegions*64.0 + x;
        int r = y/64 * m_nbRegions*4096 + y%64 * 64 + x/64 * 4096 + x%64;
        //printf("%d %d - %d\n",x,y,r);
        return r;
    }

    Mesh m_mesh;
    int m_nbRegions;
    int m_sizeRegion = 64*64;
    std::vector<vec2> m_min_max;
};

#endif
