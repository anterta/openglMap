
#include "billboards.h"
#include "gKit/wavefront.h"

void Billboards::initialiser() {                
    m_mesh = read_mesh("data/billboard/billboard.obj");
    m_texture = read_texture(0, "data/billboard/arbre.png");
    m_program = read_program("src/shader/billboards_remap.glsl");
    program_print_errors(m_program);
    m_multi_draw.initialiser(m_mesh);
}

void Billboards::createBuffers() {
    m_multi_draw.createBuffer("src/shader/indirect_cull.glsl"); 
    m_mesh.release();
}

void Billboards::show(Transform& p, Transform& v, vec3 skyColor) {
    m_multi_draw.bindBuffers(p*v);
    glUseProgram(m_program);
    program_uniform(m_program, "vpMatrix", p*v);
    program_uniform(m_program, "inverseMatrix", v.inverse());
    program_uniform(m_program, "skyColor", skyColor );

    program_use_texture(m_program, "texture0", 0, m_texture);
    m_multi_draw.multiDraw();
}

void Billboards::showForShadow(Transform& p, Transform& v, GLuint program) {
    m_multi_draw.bindBuffers(p*v);
    glUseProgram(program);
    program_use_texture(program, "texture0", 0, m_texture);
    m_multi_draw.multiDraw();
}