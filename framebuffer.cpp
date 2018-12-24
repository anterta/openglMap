
#include "framebuffer.h"

#include <iostream>
#include "gKit/window.h"


bool Framebuffer::initialiseFrameBuffer(int width, int height, bool depth) {
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
    return check();
}

void Framebuffer::bindFrameBuffer(char col){
    glBindTexture(GL_TEXTURE_2D, 0);
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

void Framebuffer::bindFrameBuffer(vec3 skyColor){
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer);
    glViewport(0, 0, m_frameBuffer_width, m_frameBuffer_height);
    glClearColor(skyColor.x, skyColor.y, skyColor.z, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Framebuffer::show() {
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

void Framebuffer::passe2() {
    // configure l'unite 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    unbindCurrentFrameBuffer();
}

    
void Framebuffer::unbindCurrentFrameBuffer() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(0, 0, window_width(), window_height());
}
     
     
GLuint Framebuffer::createFrameBuffer() {
    /* Vérification d'un éventuel ancien FBO
    if(glIsFramebuffer(m_frameBuffer) == GL_TRUE)
        glDeleteFramebuffers(1, &m_frameBuffer);*/
        
    glGenFramebuffers(1,&m_frameBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    
    return m_frameBuffer;
}


GLuint Framebuffer::createDepthFrameBuffer() {
    /* Vérification d'un éventuel ancien FBO
    if(glIsFramebuffer(m_frameBuffer) == GL_TRUE)
        glDeleteFramebuffers(1, &m_frameBuffer);*/
        
    glGenFramebuffers(1,&m_frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    glDrawBuffer(GL_NONE);
    //glReadBuffer(GL_NONE);
    
    return m_frameBuffer;
}

GLuint Framebuffer::createTextureAttachment( int width, int height) {
    /*
    if(glIsTexture(m_texture) == GL_TRUE)
        glDeleteTextures(1, &m_texture);*/

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
    
GLuint Framebuffer::createDepthTextureAttachment(int width, int height){
    /*
    if(glIsRenderbuffer(m_texture) == GL_TRUE)
        glDeleteRenderbuffers(1, &m_texture);*/

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

bool Framebuffer::check() {
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Erreur : le FBO est mal construit" << std::endl;
        return false;
    }
    return true;
}