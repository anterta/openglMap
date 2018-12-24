
#ifndef _SUN_H
#define _SUN_H

#include "framebuffer.h"
#include "terrain.h"

#include "gKit/orbiter.h"

class Sun
{
	public:
	Sun() {}
	~Sun() {
		release_program(m_program_cull);
		release_program(m_texture_program);
	}

	void rotation(float x, float y) { m_sun.rotation(x,y); }

	bool initialise(const Point pmin, const Point pmax);

	void passe1(Terrain &terrain, Billboards &billboards);

	void showFramebuffer() { m_framebuffer.show(); }

	void parametrerPasse2(GLuint &program);

	void addMesh(Mesh objet) { m_mesh = objet; }

	void drawSun(Transform view, Transform projection);

	Transform view() {
		return Lookat(m_sun.position(),m_lookat,Vector(-1,0,0));
	}

	vec3 skyColor() {
		return m_skyColor;
	}	
	
	protected:
	Orbiter m_sun;
	Mesh m_mesh;
	GLuint m_program_cull;
	GLuint m_texture_program;
	Framebuffer m_framebuffer;
	int m_frameBuffer_width = 1024;
	int m_frameBuffer_height = 1024;
	Transform m_ortho;
	Point m_lookat;
	vec3 m_skyColor = vec3(.7,.35,.1);

	private:
	Transform Ortho( const float left, const float right, const float bottom, const float top, const float znear, const float zfar )
	{
		float tx= - (right + left) / (right - left);
		float ty= - (top + bottom) / (top - bottom);
		float tz= - (zfar + znear) / (zfar - znear);
		
		return Transform(
			2.f / (right - left),	0,						0,						tx,
			0,						2.f / (top - bottom),	0,						ty, 
			0,						0,						-2.f / (zfar - znear),	tz,
			0,						0,						0,						1);
	}

	
};

#endif
