
#ifndef _MULTI_DRAW_H
#define _MULTI_DRAW_H

#include "gKit/uniforms.h"
#include "gKit/mesh.h"
#include "gKit/program.h"

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

class Multi_Draw
{
    public:
        Multi_Draw() {}
        ~Multi_Draw() {
            release_program(m_program_cull);

            glDeleteBuffers(1, &m_model_buffer);
            glDeleteBuffers(1, &m_indirect_buffer);
            glDeleteBuffers(1, &m_parameter_buffer);
            glDeleteBuffers(1, &m_remap_buffer);
            glDeleteBuffers(1, &m_object_buffer);
            glDeleteVertexArrays(1, &m_vao);
        }

        void initialiser(Mesh& mesh);

        void addPos(Vector p, float scale = 1);
        
        void createBuffer(const char* path_program_cull);

        void bindBuffers( const Transform& mvp );

        void multiDraw();

    public:
        Point m_pmin, m_pmax;
        unsigned int m_vertex_count;
        Mesh m_mesh;
        std::vector<Transform> m_multi_model;
        std::vector<Object> m_objects;

        GLuint m_vao;
        GLuint m_program_cull;
        GLuint m_model_buffer;
        GLuint m_indirect_buffer;
        GLuint m_parameter_buffer;
        GLuint m_remap_buffer;
        GLuint m_object_buffer;
};
#endif
