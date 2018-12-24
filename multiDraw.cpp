
#include "multiDraw.h"

void Multi_Draw::initialiser(Mesh& mesh) {
    mesh.bounds(m_pmin,m_pmax);
    m_vertex_count= mesh.vertex_count();
    m_mesh = mesh;
}

void Multi_Draw::addPos(Vector p, float scale) {
    m_multi_model.push_back(Translation(p)*Scale(scale,scale,scale));
    m_objects.push_back( {m_pmin*scale + p, m_vertex_count, m_pmax*scale + p, 0} );
}

void Multi_Draw::createBuffer(const char* path_program_cull) {
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    m_vao= m_mesh.create_buffers(/* texcoords */ true, /* normals */ false, /* colors */ false);

    m_program_cull= read_program(path_program_cull);  // tests de visibilite
    program_print_errors(m_program_cull);

    // transformations des objets
    glGenBuffers(1, &m_model_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_model_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Transform) * m_multi_model.size(), m_multi_model.data(), GL_DYNAMIC_DRAW);
    
    // objets a tester
    glGenBuffers(1, &m_object_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_object_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Object) * m_objects.size(), m_objects.data(), GL_DYNAMIC_DRAW);
    
    // re-indexation des objets visibles
    glGenBuffers(1, &m_remap_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_remap_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * m_objects.size(), nullptr, GL_DYNAMIC_DRAW);
    
    // parametres du multi draw indirect
    glGenBuffers(1, &m_indirect_buffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirect_buffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(IndirectParam) * m_objects.size(), nullptr, GL_DYNAMIC_DRAW);   
    
    // nombre de draws de multi draw indirect count 
    glGenBuffers(1, &m_parameter_buffer);
    glBindBuffer(GL_PARAMETER_BUFFER_ARB, m_parameter_buffer);
    glBufferData(GL_PARAMETER_BUFFER_ARB, sizeof(int), nullptr, GL_DYNAMIC_DRAW);
}

void Multi_Draw::bindBuffers( const Transform& mvp ) {
    glBindVertexArray(m_vao);
    glUseProgram(m_program_cull);
    
    // uniforms...
    program_uniform(m_program_cull, "mvpMatrix", mvp);

    // storage buffers...
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_object_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_remap_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_indirect_buffer);
    
    // compteur
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_parameter_buffer);
    // remet le compteur a zero
    unsigned int zero= 0;
    glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);
    // ou
    // glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int), &zero);

    // nombre de groupes de shaders
    int n= m_objects.size() / 256;
    if(m_objects.size() % 256)
        n= n +1;
    
    glDispatchCompute(n, 1, 1);
    
    // etape 2 : attendre le resultat
    glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
}

void Multi_Draw::multiDraw() {
    // storage buffers...
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_model_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_remap_buffer);
    
    // parametres du multi draw...
    glBindBuffer(GL_PARAMETER_BUFFER_ARB, m_parameter_buffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirect_buffer);

    glMultiDrawArraysIndirectCountARB(m_mesh.primitives(), 0, 0, m_objects.size(), 0);
}