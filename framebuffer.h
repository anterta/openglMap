
#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

class Framebuffer
{
    public:
    Framebuffer() {};

	void release() {
        glDeleteFramebuffers(1,&m_frameBuffer);
        glDeleteTextures(1,&m_texture);
	}

	void show() {
		// montrer le resultat de la passe 1 copie le framebuffer sur la fenetre
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_frameBuffer);
		unbindCurrentFrameBuffer();
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glBlitFramebuffer(
			0, 0, m_frameBuffer_width, m_frameBuffer_height,        // rectangle origine dans READ_FRAMEBUFFER
			0, 0, m_frameBuffer_width, m_frameBuffer_height,        // rectangle destination dans DRAW_FRAMEBUFFER
			GL_COLOR_BUFFER_BIT, GL_LINEAR);                        // ne copier que la couleur (+ interpoler)
	}

    void passe2() {
		// configure l'unite 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texture);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		unbindCurrentFrameBuffer();
    }

     
    void unbindCurrentFrameBuffer() {//call to switch to default frame buffer
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glViewport(0, 0, window_width(), window_height());
    }
     
    bool initialiseFrameBuffer(int width, int height, bool depth = false) {
        m_frameBuffer_width = width;
        m_frameBuffer_height = height;
        if(depth) {
            m_frameBuffer = createDepthFrameBuffer();
            m_texture = createDepthTextureAttachment(width,height);
        } else {
            m_frameBuffer = createFrameBuffer();
            m_texture = createTextureAttachment(width,height);
        }
        unbindCurrentFrameBuffer();
        printf(" framebufer %i depthbuffer %i color %i \n",m_frameBuffer,m_texture,m_texture);
        return check();
    }
     
    void bindFrameBuffer(char col = 'g'){
        glBindTexture(GL_TEXTURE_2D, 0);//To make sure the texture isn't bound
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer);
        glViewport(0, 0, m_frameBuffer_width, m_frameBuffer_height);
		switch(col) {
			case 'y':
			case 'Y':
            	glClearColor(1, 1, 0, 1);
			break;
			case 'b':
			case 'B':
            	glClearColor(0, 0, 0, 1);
			break;
			default:
            	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
			break;
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void bindFrameBuffer(vec3 skyColor){
        glBindTexture(GL_TEXTURE_2D, 0);//To make sure the texture isn't bound
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer);
        glViewport(0, 0, m_frameBuffer_width, m_frameBuffer_height);
		glClearColor(skyColor.x, skyColor.y, skyColor.z, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

	GLuint getBufferTexture() {
        return m_texture;
    }
 
    private:
    GLuint createFrameBuffer() {
		// Vérification d'un éventuel ancien FBO
		if(glIsFramebuffer(m_frameBuffer) == GL_TRUE)
			glDeleteFramebuffers(1, &m_frameBuffer);
			
        glGenFramebuffers(1,&m_frameBuffer);
        //generate name for frame buffer
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer);
        //create the framebuffer
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        //indicate that we will always render to color attachment 0
        return m_frameBuffer;
    }


    GLuint createDepthFrameBuffer() {
		// Vérification d'un éventuel ancien FBO
		if(glIsFramebuffer(m_frameBuffer) == GL_TRUE)
			glDeleteFramebuffers(1, &m_frameBuffer);
			
        glGenFramebuffers(1,&m_frameBuffer);
        //generate name for frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
        //create the framebuffer
        glDrawBuffer(GL_NONE);
        //indicate that we will not color attachment
        //glReadBuffer(GL_NONE);
        return m_frameBuffer;
    }

    GLuint createTextureAttachment( int width, int height) {
		if(glIsTexture(m_texture) == GL_TRUE)
			glDeleteTextures(1, &m_texture);

		glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
                0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

		glGenerateMipmap(GL_TEXTURE_2D);
        
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_texture, 0);
        return m_texture;
    }
     
    GLuint createDepthTextureAttachment(int width, int height){
		if(glIsRenderbuffer(m_texture) == GL_TRUE)
			glDeleteRenderbuffers(1, &m_texture);

        glGenTextures(1,&m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height,
                0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
				
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_texture, 0);
        return m_texture;
    }

    bool check() {
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			release();

			std::cout << "Erreur : le FBO est mal construit" << std::endl;
			return false;
		}
		return true;
    }

    protected: 
    GLuint m_frameBuffer;
    GLuint m_texture;

	int m_frameBuffer_width;
	int m_frameBuffer_height;
};
#endif