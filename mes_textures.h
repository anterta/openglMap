
#ifndef _MES_TEXTURES_H
#define _MES_TEXTURES_H

#include "texture.h"

class MesTextures {
    public:
        MesTextures() {};

        void read_textures() {
            water_texture = read_texture(5, "data/terrain/hardened_clay_stained_blue.png");
            grass_texture = read_texture(1, "data/terrain/concrete_powder_lime.png");
            ground_texture = read_texture(2, "data/terrain/dirt_podzol_top.png");
            stone_texture = read_texture(3, "data/terrain/stone.png");
            snow_texture = read_texture(4, "data/terrain/quartz_block_bottom.png");
        }

        void blindTextures(GLuint &program) {
            GLint location;
            glBindTexture(GL_TEXTURE_2D, water_texture);
            location= glGetUniformLocation(program, "water_texture");
            glUniform1i(location, 5);

            glBindTexture(GL_TEXTURE_2D, grass_texture);
            location= glGetUniformLocation(program, "grass_texture");
            glUniform1i(location, 1);

            glBindTexture(GL_TEXTURE_2D, ground_texture);
            location= glGetUniformLocation(program, "ground_texture");
            glUniform1i(location, 2);

            glBindTexture(GL_TEXTURE_2D, stone_texture);
            location= glGetUniformLocation(program, "stone_texture");
            glUniform1i(location, 3);

            glBindTexture(GL_TEXTURE_2D, snow_texture);
            location= glGetUniformLocation(program, "snow_texture");
            glUniform1i(location, 4);
        }

    private:
    GLuint water_texture;
    GLuint grass_texture;
    GLuint ground_texture;
    GLuint stone_texture;
    GLuint snow_texture;
};

#endif
