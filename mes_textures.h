
#ifndef _MES_TEXTURES_H
#define _MES_TEXTURES_H

#include "gKit/texture.h"

class MesTextures {
	public:
		MesTextures() {}
		~MesTextures() {
        	glDeleteTextures(5, m_textures);
			glDeleteSamplers(1, &m_sampler);
		}

		void read_textures();

		void blindTextures(GLuint &program);

	private:
		void blindOneTexture(const GLuint &program, const char* texture, GLuint location, const int num);

	GLuint m_textures[5];
	GLuint m_sampler;
};

#endif
