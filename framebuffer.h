
#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

#include "gKit/vec.h"
#include "gKit/texture.h"

class Framebuffer
{
    public:
    Framebuffer() {};
    ~Framebuffer() {
        glDeleteFramebuffers(1,&m_frameBuffer);
        glDeleteTextures(1,&m_texture);
    };

    bool initialiseFrameBuffer(int width, int height, bool depth = false);
     
    void bindFrameBuffer(char col = 'g');

    void bindFrameBuffer(vec3 skyColor);

	void release();

	void show();

    void passe2();
     
    void unbindCurrentFrameBuffer();
     
	GLuint getBufferTexture() {
        return m_texture;
    }
 
    private:
    GLuint createFrameBuffer();

    GLuint createDepthFrameBuffer();

    GLuint createTextureAttachment( int width, int height);
     
    GLuint createDepthTextureAttachment(int width, int height);

    bool check();

    protected: 
    GLuint m_frameBuffer;
    GLuint m_texture;

	int m_frameBuffer_width;
	int m_frameBuffer_height;
};
#endif