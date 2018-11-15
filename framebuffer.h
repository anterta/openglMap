
#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

class Framebuffer
{
    public:
    Framebuffer() {};

	void release() {
        glDeleteRenderbuffers(1,&m_depthBuffer);
        glDeleteFramebuffers(1,&m_frameBuffer);
        glDeleteTextures(1,&m_colorTexture);
        glDeleteTextures(1,&m_depthTexture);
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
		glBindTexture(GL_TEXTURE_2D, m_colorTexture);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		unbindCurrentFrameBuffer();
    }

     
    void unbindCurrentFrameBuffer() {//call to switch to default frame buffer
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glViewport(0, 0, window_width(), window_height());
    }
     
    bool initialiseFrameBuffer(int width, int height) {
        m_frameBuffer_width = width;
        m_frameBuffer_height = height;
        m_frameBuffer = createFrameBuffer();
        m_colorTexture = createTextureAttachment(width,height);
        //m_depthTexture = createDepthTextureAttachment(width,height);
        unbindCurrentFrameBuffer();
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

	GLuint getColorTexture() {
        return m_colorTexture;
    }
     
    GLuint getDepthTexture(){
        return m_depthTexture;
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

    GLuint createTextureAttachment( int width, int height) {
		if(glIsTexture(m_colorTexture) == GL_TRUE)
			glDeleteTextures(1, &m_colorTexture);

		glGenTextures(1, &m_colorTexture);
        glBindTexture(GL_TEXTURE_2D, m_colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
                0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

		glGenerateMipmap(GL_TEXTURE_2D);
        
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_colorTexture, 0);
        return m_colorTexture;
    }
     
    GLuint createDepthTextureAttachment(int width, int height){
		if(glIsRenderbuffer(m_depthTexture) == GL_TRUE)
			glDeleteRenderbuffers(1, &m_depthTexture);

        glGenTextures(1,&m_depthTexture);
        glBindTexture(GL_TEXTURE_2D, m_depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height,
                0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
				
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthTexture, 0);
        return m_depthTexture;
    }
 
    GLuint createDepthBufferAttachment(int width, int height) {
        glGenRenderbuffers(1,&m_depthBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer);
        return m_depthBuffer;
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
    GLuint m_depthBuffer;
    GLuint m_frameBuffer;
    GLuint m_colorTexture;
    GLuint m_depthTexture;

	int m_frameBuffer_width;
	int m_frameBuffer_height;
};
#endif