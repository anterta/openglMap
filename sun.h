
#ifndef _SUN_H
#define _SUN_H

#include "camera.h"
#include "mes_textures.h"

class Sun
{
	public:
	Sun() {};
	
	void positionne(const Point pmin, const Point pmax) {
		m_sun.lookat(pmin, pmax);
		m_sun.rotation(-70.0, 0.0);
	}

	void rotation(float x, float y) {
		m_sun.rotation(x,y);
	}

	bool createFramebuffer() {
		// Vérification d'un éventuel ancien FBO
		if(glIsFramebuffer(m_framebuffer) == GL_TRUE)
			glDeleteFramebuffers(1, &m_framebuffer);

		// Destruction d'une éventuelle ancienne texture
		if(glIsTexture(sun_color_buffer) == GL_TRUE)
			glDeleteTextures(1, &sun_color_buffer);

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
		

		// Destruction d'un éventuel ancien Render Buffer
		if(glIsRenderbuffer(sun_zbuffer_texture) == GL_TRUE)
			glDeleteRenderbuffers(1, &sun_zbuffer_texture);

		// etape 2 : creer aussi une texture depth, sinon pas de zbuffer...
		glGenTextures(1, &sun_zbuffer_texture);
		glBindTexture(GL_TEXTURE_2D, sun_zbuffer_texture);
		
		glTexImage2D(GL_TEXTURE_2D, 0,
			GL_DEPTH_COMPONENT, m_framebuffer_width, m_framebuffer_height, 0,
			GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
		
		// etape 3 : creer et configurer un framebuffer object
		glGenFramebuffers(1, &m_framebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebuffer);
		glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sun_color_buffer, 0);
		glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, sun_zbuffer_texture, 0);
		
		// le fragment shader ne declare qu'une seule sortie, indice 0
		GLenum buffers[]= { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buffers);
		
		// nettoyage
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		// shaders
		m_texture_program= read_program("src/tp1_part3_shaderTexture.glsl");
		program_print_errors(m_texture_program);

		// Always check that our framebuffer is ok
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			// Libération des buffers
			glDeleteFramebuffers(1, &m_framebuffer);
			glDeleteRenderbuffers(1, &sun_zbuffer_texture);

			// Affichage d'un message d'erreur et retour de la valeur false
			std::cout << "Erreur : le FBO est mal construit" << std::endl;
			return false;
		}

		return true;
	}

	void release() {
		glDeleteTextures(1, &sun_color_buffer);
		glDeleteTextures(1, &sun_zbuffer_texture);
		glDeleteFramebuffers(1, &m_framebuffer);
		glDeleteSamplers(1, &color_sampler);

		release_program(m_texture_program);
	}

	void passe1(GLuint &vao, Terrain terrain, int vertex_count) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebuffer);
		glViewport(0, 0, m_framebuffer_width, m_framebuffer_height);
		glClearColor(1, 1, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// draw "classique"
		glBindVertexArray(vao);
		glUseProgram(m_texture_program);

		int size = terrain.nbRegions();

		Transform m;
		m_lookat = Point(terrain.get(9*32,9*32));
		Transform r = RotationX(70);
		Point pmin = r( Point(-65*size,260*size,1));
		Point pmax = r( Point(65*size,-260*size,1));
		
		Transform v= Lookat(m_sun.position(),m_lookat,Vector(0,0,1));
		m_ortho = Ortho(pmin.x,pmax.x,pmin.y,pmax.y,33*size,130*size);
		Transform mvp= m_ortho * v * m;
		Transform mv= v * m;

		program_uniform(m_texture_program, "mvpMatrix", mvp);
		program_uniform(m_texture_program, "mvMatrix", mv);

		for(int i=0; i< terrain.nbRegions()*terrain.nbRegions(); i++)
			if(terrain.visbleCamera(i,mvp))
				terrain.drawRegion(i,vertex_count);
	}

	void showFramebuffer() {
		// montrer le resultat de la passe 1 copie le framebuffer sur la fenetre
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
		program_uniform(program, "zBuffer_texture", 0);     // utilise la texture configuree sur l'unite 0
					
		Transform v= Lookat(m_sun.position(),m_lookat,Vector(0,0,1));
		Transform mvp= m_ortho * v;

		program_uniform(program, "sunMvpMatrix", mvp);
		program_uniform(program, "sunInverseMatrix", v.inverse());

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

	void addMesh(Mesh objet) { m_mesh = objet; }

	void drawSun(Transform view, Transform projection) {
		Point p = m_sun.position();
		Transform t = Translation(Vector(p)) * Scale(50.0,50.0,50.0);
		draw(m_mesh,t,view,projection);
	}
	
	protected:
	Orbiter m_sun;
	Mesh m_mesh;
	GLuint m_texture_program;
	GLuint m_framebuffer;
	GLuint sun_zbuffer_texture;
	GLuint sun_color_buffer;
	GLuint color_sampler;
	int m_framebuffer_width = 512;
	int m_framebuffer_height = 512;
	Transform m_ortho;
	Point m_lookat;

	private:
	Transform Ortho( const float left, const float right, const float bottom, const float top, const float znear, const float zfar )
	{
		float tx= - (right + left) / (right - left);
		float ty= - (top + bottom) / (top - bottom);
		float tz= - (zfar + znear) / (zfar - znear);
		
		return Transform(
			2.f / (right - left),                    0,                     0, tx,
							0, 2.f / (top - bottom),                     0, ty, 
			0,                                       0, -2.f / (zfar - znear), tz,
			0,                                       0,                     0, 1);
	}
};

#endif
