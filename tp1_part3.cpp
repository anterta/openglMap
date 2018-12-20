
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
        m_terrain.make_terrain("data/Clipboard02.png", m_nbRegions);

        Point pmin, pmax;
        m_terrain.bounds(pmin, pmax);
        m_camera = Camera(  Vector(m_terrain.get(m_nbRegions*32,m_nbRegions*32)),
                            Vector(0,1,0),
                            Vector(1,0,0), 0.1*m_nbRegions, 0.05*m_nbRegions);
                 
        m_sun.positionne(pmin,pmax);
        
        if(!m_sun.initialise())  return -1;

        m_textures.read_textures();

        // shader program        
        m_program= read_program("src/shader_indirect_remap.glsl");        // affichage des objets visibles
        program_print_errors(m_program);
                
        // etat openGL par defaut
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
        
        glClearDepth(1.f);                          // profondeur par defaut
        glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
        glEnable(GL_DEPTH_TEST);                    // activer le ztest

        m_water.initialiser(m_nbRegions);
        return 0;
    }
    
    // destruction des objets de l'application
    int quit( )
    {
        release_program(m_program);
        m_terrain.release();
        m_sun.release();
        return 0;
    }

    int update( const float time, const float delta )
    {
        m_sun.rotation(0, 0.001*m_nbRegions*m_nbRegions);
        m_water.move();
        return 0;
    }

    // dessiner une nouvelle image
    int render( )
    {
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Transform p = Perspective(70.0, (double) window_width() / window_height(), 0.1, 150.0);
        Transform v = m_camera.lookAt();
        
        m_camera.deplacer();
        m_terrain.hauteurCamera(m_camera);

        m_sun.passe1(m_terrain);

        // Reflection
        v = m_camera.lookAt(Vector(0.0,2.0*(m_camera.position().y-float(m_nbRegions)*0.2113),0.0),true);
		m_terrain.bindBuffers(  p * v );
		
        glUseProgram(m_program);
        m_textures.blindTextures(m_program);

        m_sun.parametrerPasse2(m_program);

        m_water.bindReflectionFrameBuffer(m_program,m_camera,m_sun);
                
        m_terrain.multiDraw(m_program, p, v);

        m_water.unbindReflectionFrameBuffer();

        if(key_state('e')) {
            m_water.showReflection();
        } else if(key_state('a')) {
            m_water.showRefraction();
        } else {
            vec3 skyColor = m_sun.skyColor();
            glClearColor(skyColor.x, skyColor.y, skyColor.z, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            v = m_camera.lookAt();
            m_terrain.bindBuffers( p * v );

            glUseProgram(m_program);
            m_sun.parametrerPasse2(m_program);
                    
            m_terrain.multiDraw(m_program, p, v);
            //m_sun.drawSun(v, p);
            m_water.show(v,p, m_camera, m_terrain, m_sun);
        }

        return 1;
    }

protected:
    int m_nbRegions;
    Camera m_camera;
    Sun m_sun;
    GLuint m_program;
    Water m_water;
    Terrain m_terrain;
    MesTextures m_textures;

    Billboards m_billboards;
};


int main( int argc, char **argv )
{
    TP tp;
    tp.run();
    
    return 0;
}
