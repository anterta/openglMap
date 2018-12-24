
#include "water.h"
#include "gKit/wavefront.h"
#include "gKit/draw.h"


bool Water::initialiser(int nbRegions) {
    heightWater = float(nbRegions)*0.2113;
    if( !m_reflection.initialiseFrameBuffer(REFLECTION_WIDTH,REFLECTION_HEIGHT))
        return false;

    m_mesh = read_mesh("data/carre.obj");

    m_water_program= read_program("src/shader/water.glsl");
    program_print_errors(m_water_program);

    dudvMap = read_texture(0, "data/terrain/waterDUDV.png");
    normalMap = read_texture(0, "data/terrain/waterNormalMap.png");
    m_move = 0.;
    
    // cree le sampler
    glGenSamplers(1, &m_sampler);
    glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(m_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    return true;
}

Transform Water::bindReflectionFrameBuffer(GLuint &program, Camera &cam, Sun &sun) {
    vec3 skyColor = sun.skyColor();
    m_reflection.bindFrameBuffer(skyColor);
    glEnable(GL_CLIP_DISTANCE0);
    program_uniform(program, "clip", 1);
    Point p = cam.getPosition();
    return cam.lookAt(Vector(0.0,2.0*(p.y-heightWater),0.0),true);
}

Transform Water::inverseCam(Camera &cam) {
    Point p = cam.getPosition();
    return cam.lookAt(Vector(0.0,2.0*(p.y-heightWater),0.0),true);
}

void Water::unbindReflectionFrameBuffer() {
    m_reflection.unbindCurrentFrameBuffer();
    glDisable(GL_CLIP_DISTANCE0);
}

void Water::putTexture(GLuint &program) {
    program_uniform(program, "move", m_move);
    program_uniform(program, "waveHeight", m_waveHeight);
    program_uniform(program, "waveWidth", m_waveWidth);

    GLint location;
    location= glGetUniformLocation(program, "reflectionTexture");
    glUniform1i(location, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,  m_reflection.getBufferTexture());
    glBindSampler(0, m_sampler);

    location= glGetUniformLocation(program, "dudvMap");
    glUniform1i(location, 1);
    glActiveTexture(GL_TEXTURE0 +1);
    glBindTexture(GL_TEXTURE_2D, dudvMap);
    glBindSampler(2, m_sampler);

    location= glGetUniformLocation(program, "normalMap");
    glUniform1i(location, 2);
    glActiveTexture(GL_TEXTURE0 +2);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    glBindSampler(3, m_sampler);
}

void Water::move() {
        m_move += m_speed;
    if(m_move>1)
        m_move -= 1;
}

void Water::show(Transform view, Transform projection, Camera &cam, Terrain &terrain, Sun &sun) {//, std::vector<int> regions) {
    int nbRegions = terrain.nbRegions();
    glUseProgram(m_water_program);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    putTexture(m_water_program);
    vec4 position_sun = (projection*view*sun.view().inverse())( vec4(0,0,0,1));
    program_uniform(m_water_program, "lightPos", position_sun);

    
    vec3 p = terrain.get(nbRegions*32,nbRegions*32);
    Transform t = Translation(Vector(1,1+nbRegions*.2112,1)) * Scale(nbRegions*64,1,-64*nbRegions);
    program_uniform(m_water_program, "mvpMatrix", projection*view*t);
    program_uniform(m_water_program, "inverseMatrix", view.inverse());
    program_uniform(m_water_program, "camOrient", (float)cam.getPhi());
    program_uniform(m_water_program, "skyColor", sun.skyColor());
    draw(m_mesh,m_water_program);/*
    //printf("test %d\n", regions.size());
    for(int i=0; i<regions.size(); i++)
        if(m_min_max[regions[i]].x <= nbRegions*0.2113) {
            vec3 p = m_mesh.positions()[regions[i]*m_sizeRegion];
            Transform t = Translation(Vector(p.x,1+nbRegions*.2112,p.z)) * Scale(64,1,-64);
            program_uniform(m_water_program, "mvpMatrix", projection*view*t);
            draw(m_water,m_water_program);
        }*/
}
