
#include "orbiter.h"
#include "program.h"
#include "mes_textures.h"
#include "terrain.h"
#include "sun.h"
#include "camera.h"
#include "water.h"

#include "app_time.h"


class TP : public AppTime
{
public:
    TP( ) : AppTime(1024, 640) {}
    
    int init( )
    {
        //  verifie l'existence des extensions
        if(!GLEW_ARB_indirect_parameters) {
            printf("GL_ARB_indirect_parameters OFF\n");
            return -1;
        }
        
        if(!GLEW_ARB_shader_draw_parameters) {
            printf("GL_ARB_shader_draw_parameters OFF\n");
            return -1;
        }
        
        m_nbRegions = 10;
        
        // charge le cube et le terrain
        m_cube= read_mesh("data/cube.obj");
        if(m_cube == Mesh::error()) return -1;
        m_vertex_count= m_cube.vertex_count();

        Point pmin, pmax;
        m_cube.bounds(pmin, pmax);
        m_terrain.make_terrain("data/Clipboard02.png", m_nbRegions, pmin, pmax, m_vertex_count);

        m_terrain.bounds(pmin, pmax);
        vec3 t = m_terrain.get(m_nbRegions*32,m_nbRegions*32);
        printf("%f %f %f\n",t.x,t.y,t.z);
        m_camera = Camera(  Vector(t.x,t.y+10,t.z),
                            Vector(0,1,0),
                            Vector(1,0,0), 0.1*m_nbRegions, 0.05*m_nbRegions);
                            
        //#if 0
        m_sun.positionne(pmin,pmax);
        m_sun.addMesh(m_cube);
        
        if(m_sun.initialise())  {
            printf("Ok\n");
        }
        
        // cree les buffers et le vao
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        
        // buffer : positions + normals
        glGenBuffers(1, &m_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
        
        size_t size= m_cube.vertex_buffer_size() + m_cube.texcoord_buffer_size();
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
        
        // configure l'attribut 0, vec3 position
        size= m_cube.vertex_buffer_size();
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, m_cube.vertex_buffer());
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        // configure l'attribut 1, vec2 textcoord
        size_t offset= size;
        //offset= offset + size;
        size= m_cube.texcoord_buffer_size();
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, m_cube.texcoord_buffer());
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *) offset);
        glEnableVertexAttribArray(1);

        m_textures.read_textures();
                    
        m_terrain.createBuffer(m_buffer);
        
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // shaders
        //m_program= read_program("src/tp1_part3_shader.glsl");
        //program_print_errors(m_program);
        //#endif

        #if 1
        //m_terrain.createBuffer(m_buffer);
        m_vao= m_cube.create_buffers(/* texcoords */ true, /* normals */ false, /* colors */ false);

        // shader program        
        m_program= read_program("src/shader_indirect_remap.glsl");        // affichage des objets visibles
        program_print_errors(m_program);
        #endif
                
        // etat openGL par defaut
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
        
        glClearDepth(1.f);                          // profondeur par defaut
        glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
        glEnable(GL_DEPTH_TEST);                    // activer le ztest

        m_water.initialiser(m_nbRegions);
        m_time = 0;
        return 0;
    }
    
    // destruction des objets de l'application
    int quit( )
    {
        release_program(m_program);
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_buffer);
        m_cube.release();
        m_terrain.release();
        m_sun.release();
        return 0;
    }

    int update( const float time, const float delta )
    {
        m_sun.rotation(0, 0.0005*m_nbRegions*m_nbRegions);
        m_water.move();
        return 0;
    }

    float uniformRandomInRange(float min, float max) {
        assert(min < max);
        double n = (double) rand() / (double) RAND_MAX;
        double v = min + n * (max - min);
        return v;
    }

    // dessiner une nouvelle image
    int render( )
    {
        #if 0
        glBindVertexArray(m_vao);
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_camera.deplacer();
        m_terrain.hauteurCamera(m_camera);
        
        m_sun.passe1(m_terrain, m_vertex_count); 

        if(key_state('d')) {
            // montrer le resultat de la passe 1 copie le framebuffer sur la fenetre
            m_sun.showFramebuffer();
        } else {
            // Refraction
            glUseProgram(m_program);
            glClearColor(0.2f, 0.2f, 0.2f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            m_textures.blindTextures(m_program);

            m_sun.parametrerPasse2(m_program);

            m_water.bindRefractionFrameBuffer(m_program);
            Transform v= m_camera.lookAt();
            Transform p= Perspective(70.0, (double) window_width() / window_height(), 0.1, 1500.0);

            
            std::vector<int> regions = m_terrain.render(m_program, v, p, m_vertex_count);
            m_water.unbindRefractionFrameBuffer();

            // Reflection
            glUseProgram(m_program);
            glClearColor(0.2f, 0.2f, 0.2f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            m_sun.parametrerPasse2(m_program);

            v= m_water.bindReflectionFrameBuffer(m_program, m_camera);
            m_terrain.render(m_program, v, p, m_vertex_count, regions);
            m_water.unbindReflectionFrameBuffer();


            if(key_state('e')) {
                m_water.showReflection();
            } else if(key_state('a')) {
                m_water.showRefraction();
            } else {
                glUseProgram(m_program);

                m_sun.parametrerPasse2(m_program);

                Transform v= m_camera.lookAt();

                m_terrain.render(m_program, v, p, m_vertex_count);                

                m_sun.drawSun(v, p);
                m_water.show(v,p, m_camera, m_terrain, m_sun, regions);
            }
        }
        #endif

        #if 1
        glBindVertexArray(m_vao);
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_camera.deplacer();
        m_terrain.hauteurCamera(m_camera);

        m_sun.passe1(m_terrain, m_cube); 

        if(key_state('d')) {
            // montrer le resultat de la passe 1 copie le framebuffer sur la fenetre
            m_sun.showFramebuffer();
        } else {
            glClearColor(0.2f, 0.2f, 0.2f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                           
            Transform p = Perspective(70.0, (double) window_width() / window_height(), 0.1, 1500.0);
            Transform v = m_camera.lookAt();
            
            m_terrain.bindBuffers( p * v );

            glUseProgram(m_program);
            m_textures.blindTextures(m_program);
            m_sun.parametrerPasse2(m_program);
            
            // uniforms...
            program_uniform(m_program, "vpMatrix", p*v);
            program_uniform(m_program, "viewMatrix", v);
            program_uniform(m_program, "nbCubes", m_nbRegions*64); 
                    
            m_terrain.multiDraw(m_cube);
            m_sun.drawSun(v, p);
        }
        #endif

        


        return 1;
    }

protected:
    Transform m_model;
    int m_nbRegions;
    Camera m_camera;
    Sun m_sun;
    GLuint m_vao;
    GLuint m_buffer;
    GLuint m_program;
    int m_vertex_count;
    int m_instance_count;
    float m_time;
    Water m_water;
    Terrain m_terrain;
    MesTextures m_textures;

    Mesh m_cube;
};


int main( int argc, char **argv )
{
    TP tp;
    tp.run();
    
    return 0;
}
