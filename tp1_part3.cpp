
#include "orbiter.h"
#include "program.h"
#include "mes_textures.h"
#include "terrain.h"
#include "sun.h"
#include "camera.h"

#include "app_time.h"

class TP : public AppTime
{
public:
    TP( ) : AppTime(1024, 640) {}
    
    int init( )
    {
        m_nbRegions = 10;
        
        // charge le cube et le terrain
        Mesh mesh= read_mesh("data/flat_bbox.obj");
        if(mesh == Mesh::error()) return -1;
        m_terrain.make_terrain("data/Clipboard02.png", m_nbRegions);

        Point pmin, pmax;
        m_terrain.bounds(pmin, pmax);
        vec3 t = m_terrain.get(m_nbRegions*32,m_nbRegions*32);
        printf("%f %f %f\n",t.x,t.y,t.z);
        m_camera = Camera(  Vector(t.x,t.y+30,t.z),
                            Vector(0,1,0),
                            Vector(1,0,0), 0.1*m_nbRegions, 0.05*m_nbRegions);
        m_sun.positionne(pmin,pmax);
        m_sun.addMesh(mesh);
        
        if(m_sun.createFramebuffer())  {
            printf("Ok\n");
        }

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

        m_time = 0;
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

    int update( const float time, const float delta )
    {
        m_sun.rotation(0, 0.01*m_nbRegions*m_nbRegions);
        m_time += .01*m_nbRegions*m_nbRegions;
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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_camera.deplacer(m_terrain);
        
        m_sun.passe1(m_vao, m_terrain, m_vertex_count); 

        if(key_state(' ')) {
            // montrer le resultat de la passe 1 copie le framebuffer sur la fenetre
            m_sun.showFramebuffer();
        } else {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glViewport(0, 0, window_width(), window_height());
            glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glBindVertexArray(m_vao);
            glUseProgram(m_program);
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

            Transform m= m_model;
            Transform v= m_camera.lookAt();
            Transform p= Perspective(70.0, (double) window_width() / window_height(), 0.1, 1500.0);
            Transform mvp= p * v * m;
            
            program_uniform(m_program, "mvpMatrix", mvp);
            program_uniform(m_program, "vMatrix", v);
		    program_uniform(m_program, "inverseMatrix", v.inverse());
            program_uniform(m_program, "nbCubes", m_nbRegions*64);

vec2 direction[4];
    for (int i = 0; i < 4; ++i) {
        float angle = uniformRandomInRange(-M_PI/3, M_PI/3);
        direction[i] = vec2(cos(angle),sin(angle));
    }
            program_uniform(m_program, "direction1", direction[0]);
            program_uniform(m_program, "direction2", direction[1]);
            program_uniform(m_program, "time", m_time);  

            m_textures.blindTextures(m_program);

            m_sun.parametrerPasse2(m_program);

            int nbRegionsVisibles = 0;
            std::vector<int> regions;
            for(int i=0; i< m_nbRegions*m_nbRegions; i++)
                if(m_terrain.visbleCamera(i,mvp)) {
                    m_terrain.drawRegion(i,m_vertex_count);
                    nbRegionsVisibles++;
                    regions.push_back(i);
                }
            
            m_sun.drawSun(v, p);
            m_terrain.drawWaterRegion(regions,m_vertex_count,m_vao,v,p);
            printf(" -> %d\n",nbRegionsVisibles);
        }
        return 1;
    }

protected:
    Transform m_model;
    int m_nbRegions;
    Camera m_camera;
    Sun m_sun;
    GLuint m_vao;
    GLuint m_buffer;
    GLuint m_instance_buffer;
    GLuint m_program;
    int m_vertex_count;
    int m_instance_count;
    float m_time;

    Terrain m_terrain;
    MesTextures m_textures;
};


int main( int argc, char **argv )
{
    TP tp;
    tp.run();
    
    return 0;
}
