
#include "mat.h"
#include "mesh.h"
#include "wavefront.h"

#include "orbiter.h"
#include "program.h"
#include "uniforms.h"
#include "draw.h"
#include "mes_textures.h"
#include "terrain.h"
#include "sun.h"

#include "app_time.h"

class TP : public AppTime
{
public:
    TP( ) : AppTime(1024, 640) {}
    
    int init( )
    {
        m_nbRegions = 7;
        
        // charge le cube et le terrain
        Mesh mesh= read_mesh("data/cube.obj");
        if(mesh == Mesh::error()) return -1;
        m_terrain.make_terrain("data/Clipboard02.png", m_nbRegions);

        Point pmin, pmax;
        m_terrain.bounds(pmin, pmax);
        m_camera.lookat(pmin, pmax);
        //m_sun.positionne(pmin,pmax);
        m_camera.translation(0.0, 0.05);
        m_camera.move(10.0);
        
        //m_sun.createFramebuffer();

        m_vertex_count= mesh.vertex_count();

        // cree les buffers et le vao
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        
        // buffer : positions + normals
        glGenBuffers(1, &m_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
        
        size_t size= mesh.vertex_buffer_size() + mesh.texcoord_buffer_size();
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
        
        // configure l'attribut 0, vec3 position
        size= mesh.vertex_buffer_size();
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, mesh.vertex_buffer());
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        // configure l'attribut 3, vec2 textcoord
        size_t offset= size;
        //offset= offset + size;
        size= mesh.texcoord_buffer_size();
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, mesh.texcoord_buffer());
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *) offset);
        glEnableVertexAttribArray(3);

        m_textures.read_textures();
                    
        m_terrain.createBuffer(m_instance_buffer);
        
        mesh.release();
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // shaders
        m_program= read_program("src/tp1_part3_shader.glsl");
        program_print_errors(m_program);
                
        // etat openGL par defaut
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
        
        glClearDepth(1.f);                          // profondeur par defaut
        glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
        glEnable(GL_DEPTH_TEST);                    // activer le ztest

        return 0;
    }
    
    // destruction des objets de l'application
    int quit( )
    {
        release_program(m_program);
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_buffer);
        glDeleteBuffers(1, &m_instance_buffer);
        m_sun.release();
        return 0;
    }
    
    // dessiner une nouvelle image
    int render( )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // deplace la camera
        int mx, my;
        unsigned int mb= SDL_GetRelativeMouseState(&mx, &my);
        if(mb & SDL_BUTTON(2))              // le bouton du milieu est enfonce
            m_camera.rotation(mx, my);
        else if(mb & SDL_BUTTON(3))         // le bouton droit est enfonce
            m_camera.move(mx);
        else if(mb & SDL_BUTTON(1)) {       // le bouton gauche est enfonce
            m_camera.translation((float) mx / (float) window_width(), 0.0);
            m_camera.move(my);
        }
        
        //m_sun.passe1(m_vao, m_program, m_terrain, m_vertex_count,m_textures); 

        if(key_state(' '))
        {
            /* montrer le resultat de la passe 1
                copie le framebuffer sur la fenetre
             */
            m_sun.showFramebuffer();
        } else {/*
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glViewport(0, 0, window_width(), window_height());
            glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/

            glBindVertexArray(m_vao);
            glUseProgram(m_program);
            
            Transform m= m_model;
            Transform v= m_camera.view();
            Transform p= m_camera.projection(window_width(), window_height(), 45);
            Transform mvp= p * v * m;
            Transform mv= v * m;
            
            program_uniform(m_program, "mvpMatrix", mvp);
            program_uniform(m_program, "mvMatrix", mv);
            program_uniform(m_program, "normalMatrix", mv.normal());
            program_uniform(m_program, "nbCubes", m_nbRegions*64);        

            m_textures.blindTextures(m_program);

            //m_sun.parametrerPasse2(m_program);

            int nbRegionsVisibles = 0;
            for(int i=0; i< m_nbRegions*m_nbRegions; i++)
                if(m_terrain.visbleCamera(i,mvp)) {
                    m_terrain.drawRegion(i,m_vertex_count);
                    nbRegionsVisibles++;
                }
            
            printf(" -> %d\n",nbRegionsVisibles);
        }
        return 1;
    }

protected:
    Transform m_model;
    int m_nbRegions;
    Orbiter m_camera;
    Sun m_sun;
    GLuint m_vao;
    GLuint m_buffer;
    GLuint m_instance_buffer;
    GLuint m_program;
    int m_vertex_count;
    int m_instance_count;

    Terrain m_terrain;
    MesTextures m_textures;
};


int main( int argc, char **argv )
{
    TP tp;
    tp.run();
    
    return 0;
}
