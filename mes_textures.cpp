
#include "mes_textures.h"

void MesTextures::read_textures() {
    m_textures[0]	= read_texture(0, "data/terrain/grass_top.png");
    m_textures[1]	= read_texture(0, "data/terrain/grass_side.png");
    m_textures[2]	= read_texture(0, "data/terrain/dirt_podzol_top.png");
    m_textures[3]	= read_texture(0, "data/terrain/stone.png");
    m_textures[4]	= read_texture(0, "data/terrain/quartz_block_bottom.png");        

    // cree le sampler
    glGenSamplers(1, &m_sampler);
    glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(m_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
}

void MesTextures::blindTextures(GLuint &program) {
    GLint location;
    blindOneTexture(program, "grass_top_texture", location, 1);
    blindOneTexture(program, "grass_side_texture", location, 2);
    blindOneTexture(program, "ground_texture", location, 3);
    blindOneTexture(program, "stone_texture", location, 4);
    blindOneTexture(program, "snow_texture", location, 5);
}

void MesTextures::blindOneTexture(const GLuint &program, const char* texture, GLuint location, const int num) {
    location= glGetUniformLocation(program, texture);
    glUniform1i(location, num);
    glActiveTexture(GL_TEXTURE0 + num);
    glBindTexture(GL_TEXTURE_2D, m_textures[num-1]);
    glBindSampler(num, m_sampler);
}
