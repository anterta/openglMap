
#ifndef _MES_TEXTURES_H
#define _MES_TEXTURES_H

#include "texture.h"

class MesTextures {
	public:
		MesTextures() {};

		void read_textures() {
			water_texture       = read_texture(0, "data/terrain/hardened_clay_stained_blue.png");
			grass_top_texture   = read_texture(0, "data/terrain/grass_top.png");
			grass_side_texture  = read_texture(0, "data/terrain/grass_side.png");
			ground_texture      = read_texture(0, "data/terrain/dirt_podzol_top.png");
			stone_texture       = read_texture(0, "data/terrain/stone.png");
			snow_texture        = read_texture(0, "data/terrain/quartz_block_bottom.png");        
		
			// cree le sampler
			glGenSamplers(1, &sampler);
			glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			printf("%d %d %d \n", water_texture, grass_top_texture, snow_texture);
		}

		void blindTextures(GLuint &program) {
	/*
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, water_texture);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, grass_top_texture);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, grass_side_texture);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, ground_texture);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, stone_texture);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, snow_texture);*/


			GLint location;
			location= glGetUniformLocation(program, "water_texture");
			glUniform1i(location, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, water_texture);
			glBindSampler(0, sampler);

			location= glGetUniformLocation(program, "grass_top_texture");
			glUniform1i(location, 1);
	glActiveTexture(GL_TEXTURE0 + 1);
			glBindTexture(GL_TEXTURE_2D, grass_top_texture);
	glBindSampler(1, sampler);

			location= glGetUniformLocation(program, "grass_side_texture");
			glUniform1i(location, 2);
	glActiveTexture(GL_TEXTURE0 + 2);
			glBindTexture(GL_TEXTURE_2D, grass_side_texture);
	glBindSampler(2, sampler);

			location= glGetUniformLocation(program, "ground_texture");
			glUniform1i(location, 3);
	glActiveTexture(GL_TEXTURE0 + 3);
			glBindTexture(GL_TEXTURE_2D, ground_texture);
	glBindSampler(3, sampler);

			location= glGetUniformLocation(program, "stone_texture");
			glUniform1i(location, 4);
	glActiveTexture(GL_TEXTURE0 + 4);
			glBindTexture(GL_TEXTURE_2D, stone_texture);
	glBindSampler(4, sampler);

			location= glGetUniformLocation(program, "snow_texture");
			glUniform1i(location, 5);
	glActiveTexture(GL_TEXTURE0 + 5);
			glBindTexture(GL_TEXTURE_2D, snow_texture);
	glBindSampler(5, sampler);

		}

	private:
	GLuint water_texture;
	GLuint grass_top_texture;
	GLuint grass_side_texture;
	GLuint ground_texture;
	GLuint stone_texture;
	GLuint snow_texture;
	GLuint sampler;
};

#endif
