
#ifndef _SUN_H
#define _SUN_H

#include "framebuffer.h"
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

	bool initialise() {
		m_texture_program= read_program("src/tp1_part3_shaderTexture.glsl");
		program_print_errors(m_texture_program);

		return m_framebuffer.initialiseFrameBuffer(m_frameBuffer_width,m_frameBuffer_height);
	}

	void release() {
		m_framebuffer.release();

		release_program(m_texture_program);
	}

	void passe1(GLuint &vao, Terrain terrain, int vertex_count) {
		m_framebuffer.bindFrameBuffer();
		glClearColor(1, 1, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw "classique"
		glBindVertexArray(vao);
		glUseProgram(m_texture_program);

		int size = terrain.nbRegions();

		m_lookat = Point(terrain.get(9*32,9*32));
		Transform r = RotationX(70);
		Point pmin = r( Point(-65*size,260*size,1));
		Point pmax = r( Point(65*size,-260*size,1));
		
		Transform v= Lookat(m_sun.position(),m_lookat,Vector(0,0,1));
		m_ortho = Ortho(pmin.x,pmax.x,pmin.y,pmax.y,33*size,130*size);
		Transform mvp= m_ortho * v;

		program_uniform(m_texture_program, "mvpMatrix", mvp);
		program_uniform(m_texture_program, "mvMatrix", v);

		for(int i=0; i< terrain.nbRegions()*terrain.nbRegions(); i++)
			if(terrain.visbleCamera(i,mvp))
				terrain.drawRegion(i,vertex_count);

		m_framebuffer.unbindCurrentFrameBuffer();
	}

	void showFramebuffer() {
		m_framebuffer.show();
	}

	void parametrerPasse2(GLuint &program) {
		// utilise la texture attachee au framebuffer
		program_uniform(program, "zBuffer_texture", 0);     // utilise la texture configuree sur l'unite 0
					
		Transform v= Lookat(m_sun.position(),m_lookat,Vector(0,0,1));
		Transform mvp= m_ortho * v;

		program_uniform(program, "sunMvpMatrix", mvp);
		program_uniform(program, "sunInverseMatrix", v.inverse());

		m_framebuffer.passe2();
	}
/*
	int width_framebuffer() { return m_framebuffer_width; }
	int height_framebuffer() { return m_framebuffer_height; }*/

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
	Framebuffer m_framebuffer;
	int m_frameBuffer_width = 512;
	int m_frameBuffer_height = 512;
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
