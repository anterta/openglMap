
//! \file image_viewer.cpp permet de visualiser les images aux formats reconnus par gKit2 light bmp, jpg, tga, png, hdr, etc.

#include <cfloat>
#include <algorithm>

#include "app.h"
#include "widgets.h"

#include "image.h"
#include "image_io.h"
#include "image_hdr.h"

#include "program.h"
#include "uniforms.h"
#include "texture.h"


struct ImageViewer : public App
{
    ImageViewer( std::vector<const char *>& _filenames ) : App(1024, 640), m_filenames(_filenames) {}
    
    int init( )
    {
        m_width= 0;
        m_height= 0;
        
        Image image;
        for(int i= 0; i < (int) m_filenames.size(); i++)
        {
            printf("loading buffer %d...\n", i);
            
            if(is_hdr_image(m_filenames[i]))
                image= read_image_hdr(m_filenames[i]);
            else
                image= read_image(m_filenames[i]);
            
            if(image == Image::error())
                return -1;
            
            m_textures.push_back(make_texture(0, image));
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

            m_width= std::max(m_width, image.width());
            m_height= std::max(m_height, image.height());
        }
    
        // change le titre de la fenetre
        SDL_SetWindowTitle(m_window, m_filenames[0]);
        
        // redminsionne la fenetre
        SDL_SetWindowSize(m_window, m_width, m_height);
        
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        
        m_program= read_program("data/shaders/tonemap.glsl");
        program_print_errors(m_program);
        
        // 
        m_red= 1;
        m_green= 1;
        m_blue= 1;
        m_alpha= 1;
        m_gray= 0;
        m_compression= 2.2f;
        m_saturation= 1;
        m_saturation_step= 1;
        m_saturation_max= 1000;
        m_index= 0;
        m_zoom= 4;
        m_graph= 0;
        
        //
        int bins[100] = {};
        float ymin= FLT_MAX;
        float ymax= 0.f;
        for(int y= 0; y < image.height(); y++)
        for(int x= 0; x < image.width(); x++)
        {
            Color color= image(x, y);
            float y= color.r + color.g + color.b;
            if(y < ymin) ymin= y;
            if(y > ymax) ymax= y;
        }
        
        for(int y= 0; y < image.height(); y++)
        for(int x= 0; x < image.width(); x++)
        {
            Color color= image(x, y);
            float y= color.r + color.g + color.b;
            int b= (y - ymin) * 100.f / (ymax - ymin);
            if(b >= 99) b= 99;
            if(b < 0) b= 0;
            bins[b]++;
        }

        printf("range [%f..%f]\n", ymin, ymax);
        //~ for(int i= 0; i < 100; i++)
            //~ printf("%f ", ((float) bins[i] * 100.f / (m_width * m_height)));
        //~ printf("\n");
        
        float qbins= 0;
        for(int i= 0; i < 100; i++)
        {
            if(qbins > .75f)
            {
                m_saturation= ymin + (float) i / 100.f * (ymax - ymin);
                m_saturation_step= m_saturation / 40.f;
                m_saturation_max= ymax;
                break;
            }
            
            qbins= qbins + (float) bins[i] / (m_width * m_height);
        }
        
        //
        m_widgets= create_widgets();
        
        // etat openGL par defaut
        glUseProgram(0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        return 0;
    }
    
    int quit( )
    {
        glDeleteVertexArrays(1, &m_vao);
        glDeleteTextures(m_textures.size(), m_textures.data());
        
        release_program(m_program);
        release_widgets(m_widgets);
        return 0;
    }
    
    int render( )
    {
        // effacer l'image
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        if(key_state('r'))
        {
            clear_key_state('r');
            reload_program(m_program, "data/shaders/tonemap.glsl");
            program_print_errors(m_program);
        }
        
        if(key_state(SDLK_LEFT))
        {
            clear_key_state(SDLK_LEFT);
            m_index= (m_index -1 + m_textures.size()) % m_textures.size();
            
            SDL_SetWindowTitle(m_window, m_filenames[m_index]);
        }
        if(key_state(SDLK_RIGHT))
        {
            clear_key_state(SDLK_RIGHT);
            m_index= (m_index +1 + m_textures.size()) % m_textures.size();
            
            SDL_SetWindowTitle(m_window, m_filenames[m_index]);
        }
        
        int xmouse, ymouse;
        unsigned int bmouse= SDL_GetMouseState(&xmouse, &ymouse);
        
        glBindVertexArray(m_vao);
        glUseProgram(m_program);
        
        program_use_texture(m_program, "image", 0, m_textures[m_index], 0);
        program_use_texture(m_program, "image_next", 1, m_textures[(m_index +1) % m_textures.size()], 0);
        
        if(bmouse & SDL_BUTTON(1))
            program_uniform(m_program, "split", (int) xmouse);
        else
            program_uniform(m_program, "split", (int) window_width() +2);
        
        program_uniform(m_program, "channels", vec4(m_red, m_green, m_blue, m_alpha));
        program_uniform(m_program, "gray", float(m_gray));
        program_uniform(m_program, "compression", m_compression);
        program_uniform(m_program, "saturation", m_saturation);
        
        if(bmouse & SDL_BUTTON(3))
        {
            SDL_MouseWheelEvent wheel= wheel_event();
            if(wheel.y != 0)
            {
                //~ clear_wheel_event();
                m_zoom= m_zoom + float(wheel.y) / 4.f;
                if(m_zoom < .1f) m_zoom= .1f;
                if(m_zoom > 10.f) m_zoom= 10.f;
            }
        }
    
        if(bmouse & SDL_BUTTON(3))
            program_uniform(m_program, "zoom", m_zoom);
        else
            program_uniform(m_program, "zoom", 1.f);
        
        program_uniform(m_program, "center", vec2( float(xmouse) / float(window_width()), float(window_height() - ymouse +1) / float(window_height())));
        
        if(key_state('g'))
        {
            clear_key_state('g');
            m_graph= (m_graph +1) % 2;
        }

        program_uniform(m_program, "graph", int(m_graph));
        program_uniform(m_program, "line", vec2(float(window_height() - ymouse +1) / float(window_height()), float(window_height() - ymouse +1)));
        
        // dessine 1 triangle plein ecran
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        //
        if(key_state('c'))
        {
            clear_key_state('c');
            
            // change l'extension
            std::string file= m_filenames[m_index];
            size_t ext= file.rfind(".");
            if(ext != std::string::npos)
                file= file.substr(0, ext) + ".png";
            
            printf("writing '%s'...\n", file.c_str());
            screenshot(file.c_str());
        }
        
        begin(m_widgets);
            value(m_widgets, "saturation", m_saturation, 0.f, m_saturation_max*10, m_saturation_step);
            value(m_widgets, "compression", m_compression, .1f, 10.f, .1f);
        
        begin_line(m_widgets);
            button(m_widgets, "R", m_red);
            button(m_widgets, "G", m_green);
            button(m_widgets, "B", m_blue);
            button(m_widgets, "A", m_alpha);
            button(m_widgets, "gray", m_gray);
        end(m_widgets);
        
        draw(m_widgets, window_width(), window_height());
        return 1;
    }
    
protected:
    Widgets m_widgets;
    
    std::vector<const char *> m_filenames;
    std::vector<GLuint> m_textures;
    int m_width, m_height;

    GLuint m_program;
    GLuint m_vao;
    
    int m_red, m_green, m_blue, m_alpha, m_gray;
    
    float m_compression;
    float m_saturation;
    float m_saturation_step;
    float m_saturation_max;

    float m_zoom;
    int m_index;
    int m_graph;
};


int main( int argc, char **argv )
{
    if(argc == 1)
    {
        printf("usage: %s image.[bmp|png|jpg|tga|hdr]\n", argv[0]);
        return 0;
    }
    
    std::vector<const char *> options(argv +1, argv + argc);
    ImageViewer app(options);
    app.run();
    
    return 0;
}
