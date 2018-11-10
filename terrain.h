
#ifndef _TERRAIN_H
#define _TERRAIN_H

#include "mat.h"
#include "mesh.h"
#include "wavefront.h"
#include "image.h"
#include "image_io.h"
#include "uniforms.h"
#include "draw.h"
#include "program.h"

class Terrain
{
    public:
    Terrain() {};
    
    void make_terrain( const char *filename, const int nbRegions );
    void createBuffer(GLuint &buffer);
    bool visbleCamera(int i, Transform mvp);

    void drawRegion(int i, int vertex_count);
    void drawWaterRegion(std::vector<int> regions, int vertex_count, GLuint &vao, Transform view, Transform projection);
    std::vector<int> render(GLuint program, Transform view, Transform projection, int nbVertex);

    int sizeRegion() { return m_sizeRegion; }
    Mesh mesh() { return m_mesh; }
    void bounds(Point &pmin, Point &pmax) { m_mesh.bounds(pmin,pmax); }
    int nbRegions() { return m_nbRegions; }
    vec3 get(int x, int y) { return m_mesh.positions()[indice(x,y)]; }
    float getHauteur(float x, float y);

    protected:
    int indice(int x,int y) {
        return y/64 * m_nbRegions*4096 + y%64 * 64 + x/64 * 4096 + x%64;
    }
    void createMeshWater();

    Mesh m_mesh;
    Mesh m_water;
    int m_nbRegions;
    int m_sizeRegion = 64*64;
    std::vector<vec2> m_min_max;
	GLuint m_water_program;
};

#endif
