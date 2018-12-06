
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
#include "camera.h"


// representation des parametres 
struct alignas(4) IndirectParam
{
    unsigned int vertex_count;
    unsigned int instance_count;
    unsigned int first_vertex;
    unsigned int first_instance;
};
// alignement GLSL correct...

struct alignas(16) Object
{
    Point pmin;
    unsigned int vertex_count;
    Point pmax;
    unsigned int vertex_base;
};
// alignement GLSL correct...


class Terrain
{
    public:
    Terrain() {};
    
    void make_terrain( const char *filename, const int nbRegions, Point pmin, Point pmax, int &vertex_count);
    void createBuffer(GLuint &buffer);
    bool visbleCamera(int i, Transform mvp);

    void drawRegion(int i, int vertex_count);
    void pushRegionsVectorIndirect(int i, int nbVertex);
    std::vector<int> render(GLuint program, Transform view, Transform projection, int nbVertex, std::vector<int> regionsVisible = {});

    int sizeRegion() { return m_sizeRegion; }
    Mesh mesh() { return m_mesh; }
    void bounds(Point &pmin, Point &pmax) { m_mesh.bounds(pmin,pmax); }
    int nbRegions() { return m_nbRegions; }
    vec3 get(int x, int y) { return m_mesh.positions()[indice(x,y)]; }
    float getHauteur(float x, float y);
    void hauteurCamera(Camera &cam);

    void bindBuffers( const Transform& vp );
    void multiDraw(Mesh &cube);
    void release();

    protected:
    int indice(int x,int y) {
        return y/64 * m_nbRegions*4096 + y%64 * 64 + x/64 * 4096 + x%64;
    }
    void createMeshWater();

    Mesh m_mesh;
    int m_nbRegions;
    int m_sizeRegion = 64*64;
    std::vector<vec2> m_min_max;
    std::vector<Transform> m_multi_model;
    std::vector<Object> m_objects;


    GLuint m_program_cull;
    GLuint m_model_buffer;
    GLuint m_indirect_buffer;
    GLuint m_parameter_buffer;
    GLuint m_remap_buffer;
    GLuint m_object_buffer;
};

#endif
