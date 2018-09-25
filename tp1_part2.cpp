
//! \file tuto9.cpp utilisation d'un shader 'utilisateur' pour afficher un objet Mesh

#include "mat.h"
#include "mesh.h"
#include "wavefront.h"

#include "orbiter.h"
#include "program.h"
#include "uniforms.h"
#include "draw.h"
#include "image.h"
#include "image_io.h"

#include "app.h"        // classe Application a deriver


class TP : public App
{
public:
    // constructeur : donner les dimensions de l'image, et eventuellement la version d'openGL.
    TP( ) : App(1024, 640) {}
    
    int init( )
    {
        //m_objet= read_mesh("data/clipboard01.obj");
        m_objet = make_terrain("data/Clipboard02.png");

        Point pmin, pmax;
        m_objet.bounds(pmin, pmax);
        m_camera.lookat(pmin, pmax);

        // etape 1 : creer le shader program
        m_program= read_program("src/tp1_part2_color.glsl");
        program_print_errors(m_program);
        
        // etat openGL par defaut
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
        
        glClearDepth(1.f);                          // profondeur par defaut
        glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
        glEnable(GL_DEPTH_TEST);                    // activer le ztest

        return 0;   // ras, pas d'erreur
    }
    
    // destruction des objets de l'application
    int quit( )
    {
        // etape 3 : detruire le shader program
        release_program(m_program);
        m_objet.release();
        return 0;
    }
    
    // dessiner une nouvelle image
    int render( )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // deplace la camera
        int mx, my;
        unsigned int mb= SDL_GetRelativeMouseState(&mx, &my);
        if(mb & SDL_BUTTON(1))              // le bouton gauche est enfonce
            m_camera.rotation(mx, my);
        else if(mb & SDL_BUTTON(3))         // le bouton droit est enfonce
            m_camera.move(mx);
        else if(mb & SDL_BUTTON(2))         // le bouton du milieu est enfonce
            m_camera.translation((float) mx / (float) window_width(), (float) my / (float) window_height());
        
        // etape 2 : dessiner m_objet avec le shader program
        // configurer le pipeline 
        glUseProgram(m_program);
        
        // configurer le shader program
        // . recuperer les transformations
        //Transform model= RotationX(global_time() / 20);
        Transform view= m_camera.view();
        Transform projection= m_camera.projection(window_width(), window_height(), 45);
        
        // . composer les transformations : model, view et projection
        Transform mvp= projection * view;// * model;
        
        // . parametrer le shader program :
        //   . transformation : la matrice declaree dans le vertex shader s'appelle mvpMatrix
        program_uniform(m_program, "mvpMatrix", mvp);
        //   . ou, directement en utilisant openGL :
        //   int location= glGetUniformLocation(program, "mvpMatrix");
        //   glUniformMatrix4fv(location, 1, GL_TRUE, mvp.buffer());
        
        // . parametres "supplementaires" :
        //   . couleur des pixels, cf la declaration 'uniform vec4 color;' dans le fragment shader
        program_uniform(m_program, "color", vec4(1, 1, 0, 1));
        //   . ou, directement en utilisant openGL :
        //   int location= glGetUniformLocation(program, "color");
        //   glUniform4f(location, 1, 1, 0, 1);
        
        // go !
        draw(m_objet, m_program);
        
        return 1;
    }

    Mesh make_terrain( const char *filename )
    {
        Mesh mesh(GL_TRIANGLES);
        
        Image data = read_image(filename);
        
        float scale= 20;
        
    #if 0
        // solution 1 : genere un maillage directement, duplique les sommets des triangles.
        for(int y= 0; y +1 < data.height(); y++)
        for(int x= 0; x +1 < data.width(); x++)
        {
            // recupere l'altitude de 4 points voisins, dans le sens trigo
            float a= data(x, y).r * scale;
            float b= data(x +1, y).r * scale;
            float c= data(x +1, y +1).r * scale;
            float d= data(x, y +1).r * scale;
            
            // construit 2 triangles avec les 4 points
            // le terrain est constuit dans le plan xz, et y est l'altitude
            
            // rappel : la camera regarde -Z, pas +Z, il faut construire le terrain sur +x -z (et pas +x +z)
            // en gros utiliser -y au lieu de y comme coordonnee z, sinon les triangles sont "retournes"...
            mesh.texcoord(x, y).vertex(Point(x, a, -y));
            mesh.texcoord(x+1, y).vertex(Point(x +1, b, -y));
            mesh.texcoord(x+1, y+1).vertex(Point(x +1, c, -y -1));
            
            mesh.texcoord(x, y).vertex(Point(x, a, -y));
            mesh.texcoord(x+1, y+1).vertex(Point(x +1, c, -y -1));
            mesh.texcoord(x, y+1).vertex(Point(x, d, -y -1));
        }

    #else
        // solution 2 : genere un maillage indexe.
        
        // genere les positions des sommets sur la grille
        for(int y= 0; y < data.height(); y++)
        for(int x= 0; x < data.width(); x++)
        {
            // recupere l'altitude du sommet
            float a= data(x, y).r * scale;
            
            // genere la coordonnee de texture
            mesh.texcoord(float(x) / data.width(), float(y) / data.height());
            // genere la position
            mesh.vertex(x, a, -y); 
        }
        
        // genere les triangles indexes
        for(int y= 0; y +1 < data.height(); y++)
        for(int x= 0; x +1 < data.width(); x++)
        {
            // recupere les 4 voisins et genere 2 triangles
            
            // recalcule l'indice des sommets generes par la premiere etape
            int a= y * data.width() + x;
            int b= y * data.width() + (x+1);
            int c= (y+1) * data.width() + (x+1);
            int d= (y+1) * data.width() + x;
            
            mesh.triangle(a, b, c);
            mesh.triangle(a, c, d);
        }
    #endif

        return mesh;
    }

protected:
    Transform m_model;
    Mesh m_objet;
    Orbiter m_camera;
    GLuint m_texture;
    GLuint m_program;
};


int main( int argc, char **argv )
{
    TP tp;
    tp.run();
    
    return 0;
}
