
#include "sun.h"

#include "gKit/draw.h"


bool Sun::initialise(const Point pmin, const Point pmax) {
    m_sun.lookat(pmin, pmax);
    m_sun.move(-50);
    //m_sun.rotation(0.0, 90.0);

    m_program_cull= read_program("src/shader/indirect_cull.glsl");    // tests de visibilite
    program_print_errors(m_program_cull);
    
    m_texture_program= read_program("src/shader/depth.glsl");        // affichage des objets visibles
    program_print_errors(m_texture_program);

    return m_framebuffer.initialiseFrameBuffer(m_frameBuffer_width,m_frameBuffer_height,true);
}

void Sun::passe1(Terrain &terrain, Billboards &billboards) {
    m_framebuffer.bindFrameBuffer('y');

    int size = terrain.nbRegions();

    m_lookat = Point(terrain.get(10*32,10*32));
    Transform r = RotationX(70);
    Point pmin = r( Point(-65*size,260*size,1));
    Point pmax = r( Point(65*size,-260*size,1));
    float tmp = 6*64;
    Transform v= view();
    //m_ortho = Ortho(pmin.x,pmax.x,pmin.y,pmax.y,33*size,130*size);
    m_ortho = Ortho(-tmp,tmp,-tmp,tmp,900,1700);
    Transform mvp = m_ortho * v;
    terrain.bindBuffers( mvp );
    
    glUseProgram(m_texture_program);
    program_uniform(m_texture_program, "vpMatrix", mvp);

    terrain.multiDraw();
    billboards.showForShadow( m_ortho, v, m_texture_program);

    m_framebuffer.unbindCurrentFrameBuffer();
}

void Sun::parametrerPasse2(GLuint &program) {
    // utilise la texture attachee au framebuffer
    //program_uniform(program, "zBuffer_texture", 0);     // utilise la texture configuree sur l'unite 0
                
    Transform v= view();
    Transform mvp= m_ortho * v;

    float nuit = dot(vec3(0,1,0),normalize(m_sun.position()-m_lookat));
    if(nuit < 0)
        nuit=0;
    //m_skyColor = vec3(0.7 - nuit/2, .35, 3*nuit/10 + 0.1);
    //m_skyColor = vec3(0.3, 0.5, .7);
    m_skyColor = vec3(.05 + nuit*0.65,.05 + nuit*0.75,.1 + nuit*.75);

    program_uniform(program, "sunMvpMatrix", mvp);
    program_uniform(program, "sunInverseMatrix", v.inverse());
    program_uniform(program, "skyColor", m_skyColor);

    m_framebuffer.passe2();
}

void Sun::drawSun(Transform view, Transform projection) {
    Point p = m_sun.position();
    Transform t = Translation(Vector(p)) * Scale(50.0,50.0,50.0);
    draw(m_mesh,t,view,projection);
}
