
#ifndef _SUN_H
#define _SUN_H

#include "orbiter.h"
#include "mes_textures.h"

class Sun
{
    public:
    Sun() {};
    
    void positionne(const Point pmin, const Point pmax) {
        m_sun.lookat(pmin, pmax);
        m_sun.rotation(-70.0, 35.0);
    }

    bool createFramebuffer() {
        // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
        glGenFramebuffers(1, &m_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

        // Depth texture. Slower than a depth buffer, but you can sample it later in your shader
        glGenTextures(1, &sun_zbuffer_texture);
        glBindTexture(GL_TEXTURE_2D, sun_zbuffer_texture);
        glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT16, m_framebuffer_width, m_framebuffer_height, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, sun_zbuffer_texture, 0);

        glDrawBuffer(GL_NONE); // No color buffer is drawn to.

        // Always check that our framebuffer is ok
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            return false;

        return true;

/*
        // etape 1 : creer une texture couleur...
        glGenTextures(1, &sun_color_buffer);
        glBindTexture(GL_TEXTURE_2D, sun_color_buffer);
        
        glTexImage2D(GL_TEXTURE_2D, 0,
            GL_RGBA, m_framebuffer_width, m_framebuffer_height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        // ... et tous ses mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // et son sampler
        glGenSamplers(1, &color_sampler);
        
        glSamplerParameteri(color_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(color_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(color_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glSamplerParameteri(color_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        
        // etape 2 : creer aussi une texture depth, sinon pas de zbuffer...
        glGenTextures(1, &sun_zbuffer_texture);
        glBindTexture(GL_TEXTURE_2D, sun_zbuffer_texture);
        
        glTexImage2D(GL_TEXTURE_2D, 0,
            GL_DEPTH_COMPONENT, m_framebuffer_width, m_framebuffer_height, 0,
            GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
        
        // etape 2 : creer et configurer un framebuffer object
        glGenFramebuffers(1, &m_framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebuffer);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sun_color_buffer, 0);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, sun_zbuffer_texture, 0);
        
        // le fragment shader ne declare qu'une seule sortie, indice 0
        GLenum buffers[]= { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, buffers);
        
        // nettoyage
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);*/

        // shaders
        m_texture_program= read_program("src/tp1_part3_shaderTexture.glsl");
        program_print_errors(m_texture_program);
    }

    void release() {
        glDeleteTextures(1, &sun_color_buffer);
        glDeleteTextures(1, &sun_zbuffer_texture);
        glDeleteFramebuffers(1, &m_framebuffer);
        glDeleteSamplers(1, &color_sampler);

        release_program(m_texture_program);
    }

    void passe1(GLuint &vao, GLuint &program, Terrain terrain, int vertex_count, MesTextures textures) {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebuffer);
        glViewport(0, 0, m_framebuffer_width, m_framebuffer_height);
        glClearColor(1, 1, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw "classique"
        glBindVertexArray(vao);
        glUseProgram(m_texture_program);

        Transform m;
        Transform v= m_sun.view();
        Transform p= m_sun.projection(m_framebuffer_width, m_framebuffer_height, 45);
        Transform mvp= p * v * m;
        Transform mv= v * m;

        program_uniform(m_texture_program, "mvpMatrix", mvp);
        program_uniform(m_texture_program, "mvMatrix", mv);
        program_uniform(m_texture_program, "normalMatrix", mv.normal());

        for(int i=0; i< terrain.nbRegions()*terrain.nbRegions(); i++)
            if(terrain.visbleCamera(i,mvp)) {
                terrain.drawRegion(i,vertex_count);
            }
    }

    void showFramebuffer() {
            /* montrer le resultat de la passe 1
                copie le framebuffer sur la fenetre
             */
            
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glViewport(0, 0, window_width(), window_height());
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            
            glBlitFramebuffer(
                0, 0, m_framebuffer_width, m_framebuffer_height,        // rectangle origine dans READ_FRAMEBUFFER
                0, 0, m_framebuffer_width, m_framebuffer_height,        // rectangle destination dans DRAW_FRAMEBUFFER
                GL_COLOR_BUFFER_BIT, GL_LINEAR);                        // ne copier que la couleur (+ interpoler)
    }

    void parametrerPasse2(GLuint &program) {
            // utilise la texture attachee au framebuffer
            program_uniform(m_texture_program, "color_texture", 0);     // utilise la texture configuree sur l'unite 0
                        
            // configure l'unite 0
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, sun_color_buffer);
            glBindSampler(0, color_sampler);
            
            // SOIT :
            // bloque le filtrage de la texture pour n'utiliser que le mipmap 0
            //~ glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            
            // OU :
            // recalcule les mipmaps de la texture... ils sont necessaires pour afficher le cube texture, 
            // pourquoi ? un draw dans un framebuffer ne modifie que le mipmap 0, pas les autres, donc il faut les recalculer...
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, miplevels(m_framebuffer_width, m_framebuffer_height));
            glGenerateMipmap(GL_TEXTURE_2D);
    }

    int width_framebuffer() { return m_framebuffer_width; }
    int height_framebuffer() { return m_framebuffer_height; }
    
    protected:
    Orbiter m_sun;
    GLuint m_texture_program;
    GLuint m_framebuffer;
    GLuint sun_zbuffer_texture;
    GLuint sun_color_buffer;
    GLuint color_sampler;
    int m_framebuffer_width = 512;
    int m_framebuffer_height = 512;
};

#endif
