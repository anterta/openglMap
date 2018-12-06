#include "terrain.h"
#include "time.h"

void Terrain::make_terrain( const char *filename, const int nbRegions, Point pmin, Point pmax, int &vertex_count ) {
    m_nbRegions = nbRegions;

    Image data = read_image(filename);
            
    const float nbCubes = (float)nbRegions*64.0;

    float height[(int)nbCubes*(int)nbCubes];
    // genere les positions des sommets sur la grille
    float coord_scaleY = float(data.height())/nbCubes;
    float coord_scaleX = float(data.width())/nbCubes;

    for(int ry=0; ry<nbRegions; ry++)
        for(int rx=0; rx<nbRegions; rx++){
            float hmin = std::numeric_limits<float>::max();
            float hmax = std::numeric_limits<float>::min();

            for(int y= ry*64; y < (ry+1)*64; y++)
                for(int x=rx*64; x < (rx+1)*64; x++)
                {
                    // recupere l'altitude du sommet
                    float a= data.sample(x*coord_scaleX, y*coord_scaleY).r * nbCubes/20.0;
                    
                    if(a>hmax)
                        hmax=a;
                    if(a<hmin)
                        hmin=a;

                    // genere la position
                    m_mesh.vertex(x, a, -y);
                    height[indice(x,y)] = a;
                    m_multi_model.push_back(Translation(x, a, -y));
                    m_objects.push_back( {pmin + Vector(x, a, -y), unsigned(vertex_count), pmax + Vector(x, a, -y), 0} );
                }
            m_min_max.push_back(vec2(hmin,hmax));
        }

    for(int ry=0; ry<nbRegions; ry++)
        for(int rx=0; rx<nbRegions; rx++)
            for(int y= ry*64; y < (ry+1)*64; y++)
                for(int x=rx*64; x < (rx+1)*64; x++)
                {
                    float hL = 0.0;
                    float hR = 0.0;
                    float hD = 0.0;
                    float hU = 0.0;
                    if(x>0)
                        hL = height[indice(x-1,y)];
                    if(x<nbRegions*64-1)
                        hR = height[indice(x+1,y)];
                    if(y>0)
                        hD = height[indice(x,y-1)];
                    if(y<nbRegions*64-1)
                        hU = height[indice(x,y+1)];

                    m_mesh.normal(normalize(Vector(hL - hR,2.0,hD - hU)));
                }
}

void Terrain::createBuffer(GLuint &buffer) {
    #if 0
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    
    size_t size= m_mesh.vertex_buffer_size() + m_mesh.normal_buffer_size();
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
    
    // configure l'attribut 2, vec3 position
    size= m_mesh.vertex_buffer_size();
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, m_mesh.vertex_buffer());
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(2);

    // configure l'attribut 3, vec3 normal
    size_t offset= size;
    size= m_mesh.normal_buffer_size();
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, m_mesh.normal_buffer());
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *) offset);
    glVertexAttribDivisor(3, 1);
    glEnableVertexAttribArray(3);

    #endif

    #if 1
        m_program_cull= read_program("src/shader_indirect_cull.glsl");  // tests de visibilite
        program_print_errors(m_program_cull);

        // transformations des objets
        glGenBuffers(1, &m_model_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_model_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Transform) * m_multi_model.size(), m_multi_model.data(), GL_DYNAMIC_DRAW);
        
        // objets a tester
        glGenBuffers(1, &m_object_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_object_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Object) * m_objects.size(), m_objects.data(), GL_DYNAMIC_DRAW);
        
        // re-indexation des objets visibles
        glGenBuffers(1, &m_remap_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_remap_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * m_objects.size(), nullptr, GL_DYNAMIC_DRAW);
        
        // parametres du multi draw indirect
        glGenBuffers(1, &m_indirect_buffer);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirect_buffer);
        glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(IndirectParam) * m_objects.size(), nullptr, GL_DYNAMIC_DRAW);   
        
        // nombre de draws de multi draw indirect count 
        glGenBuffers(1, &m_parameter_buffer);
        glBindBuffer(GL_PARAMETER_BUFFER_ARB, m_parameter_buffer);
        glBufferData(GL_PARAMETER_BUFFER_ARB, sizeof(int), nullptr, GL_DYNAMIC_DRAW);
    #endif
    
    m_mesh.release();
}

void Terrain::bindBuffers( const Transform& mvp ) {
    glUseProgram(m_program_cull);
    
    // uniforms...
    program_uniform(m_program_cull, "mvpMatrix", mvp);

    // storage buffers...
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_object_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_remap_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_indirect_buffer);
    
    // compteur
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_parameter_buffer);
    // remet le compteur a zero
    unsigned int zero= 0;
    glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);
    // ou
    // glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int), &zero);

    // nombre de groupes de shaders
    int n= m_objects.size() / 256;
    if(m_objects.size() % 256)
        n= n +1;
    
    glDispatchCompute(n, 1, 1);
    
    // etape 2 : attendre le resultat
    glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
}

void Terrain::multiDraw(Mesh &cube) {
    // storage buffers...
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_model_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_remap_buffer);
    
    // parametres du multi draw...
    glBindBuffer(GL_PARAMETER_BUFFER_ARB, m_parameter_buffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirect_buffer);

    glMultiDrawArraysIndirectCountARB(cube.primitives(), 0, 0, m_objects.size(), 0);
}

void Terrain::release() {
    release_program(m_program_cull);

    glDeleteBuffers(1, &m_model_buffer);
    glDeleteBuffers(1, &m_indirect_buffer);
    glDeleteBuffers(1, &m_parameter_buffer);
    glDeleteBuffers(1, &m_remap_buffer);
    glDeleteBuffers(1, &m_object_buffer);
}

bool visible( const Transform& mvp, const Point& pmin, const Point& pmax ) {
    int planes[6] = { };
    
    // enumere les 8 sommets de la boite englobante
    for(unsigned int i= 0; i < 8; i++)
    {
        Point p= pmin;
        if(i & 1) p.x= pmax.x;
        if(i & 2) p.y= pmax.y;
        if(i & 4) p.z= pmax.z;
        
        // transformation du point homogene (x, y, z, w= 1)
        vec4 h= mvp(vec4(p));
        
        // teste la position du point homogene par rapport aux 6 faces de la region visible
        if(h.x < -h.w) planes[0]++;     // trop a gauche
        if(h.x > h.w) planes[1]++;      // trop a droite
        
        if(h.y < -h.w) planes[2]++;     // trop bas
        if(h.y > h.w) planes[3]++;      // trop haut
        
        if(h.z < -h.w) planes[4]++;     // trop pres
        if(h.z > h.w) planes[5]++;      // trop loin
    }
    
    // verifie si tous les sommets sont du "mauvais cote" d'une seule face, planes[i] == 8
    for(unsigned int i= 0; i < 6; i++)
        if(planes[i] == 8)
            return false;       // la boite englobante n'est pas visible
    
    // l'objet doit etre visible, ou pas, il faut aussi le test dans l'autre sens... 
    return true;
}

bool Terrain::visbleCamera(int i, Transform mvp) {
    int y = i/m_nbRegions;
    int x = i - y * m_nbRegions;
    Point pmin = Point(x*64,m_min_max[i].x,-y*64);
    Point pmax = Point((x+1)*64,m_min_max[i].y,-(y+1)*64);

    vec4 vmin = mvp(vec4(pmin));
    vec4 vmax = mvp(vec4(pmax));

    vec4 vmin2 = mvp(vec4(Point(x*64,m_min_max[i].x,-(y+1)*64)));
    vec4 vmax2 = mvp(vec4(Point((x+1)*64,m_min_max[i].y,-y*64)));
    
    return visible(mvp,pmin,pmax);/* && (
            (((vmin.x > -vmin.w || vmax.x > -vmax.w) && (vmin.x < vmin.w || vmax.x < vmax.w)) && 
            ((vmin.y > -vmin.w || vmax.y > -vmax.w) && (vmin.y < vmin.w || vmax.y < vmax.w)) && 
            ((vmin.z > -vmin.w || vmax.z > -vmax.w) && (vmin.z < vmin.w || vmax.z < vmax.w)))
        || (((vmin2.x > -vmin2.w || vmax2.x > -vmax2.w) && (vmin2.x < vmin2.w || vmax2.x < vmax2.w)) && 
            ((vmin2.y > -vmin2.w || vmax2.y > -vmax2.w) && (vmin2.y < vmin2.w || vmax2.y < vmax2.w)) && 
            ((vmin2.z > -vmin2.w || vmax2.z > -vmax2.w) && (vmin2.z < vmin2.w || vmax2.z < vmax2.w))));*/
}

float Terrain::getHauteur(float x, float y) {
    if(x==(int)x && y==(int)y) {
        return get((int)x,(int)y).y;
    } else if(x==(int)x) {
        int minY = (int)y;
        int maxY = minY+1;
        float h1 = get((int)x,minY).y;
        float h2 = get((int)x,maxY).y;
        float d = y - minY;
        return h1 + d*(h1-h2);
    } else if(y==(int)y){
        int minX = (int)x;
        int maxX = minX+1;
        float h1 = get(minX,(int)y).y;
        float h2 = get(maxX,(int)y).y;
        float d = x - minX;
        return h1 + d*(h1-h2);
    }

    int minX = (int)x;
    int maxX = minX+1;
    int minY = (int)y;
    int maxY = minY+1;
    float h1 = get(minX,minY).y;
    float h2 = get(maxX,maxY).y;
    x -= (float)minX;
    y -= (float)minY;
    float d = sqrt(x*x+y*y)/sqrt(2.0);
    return h1 + d*(h1-h2);
}

void Terrain::drawRegion(int i, int vertex_count) {
    glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, vertex_count, m_sizeRegion, i*m_sizeRegion);
}

std::vector<int> Terrain::render(GLuint program, Transform view, Transform projection, int nbVertex, std::vector<int> regionsVisible) {
    Transform mvp= projection * view;

    program_uniform(program, "mvpMatrix", mvp);
    program_uniform(program, "vMatrix", view);
    program_uniform(program, "inverseMatrix", view.inverse());
    program_uniform(program, "nbCubes", m_nbRegions*64); 

    if(regionsVisible.size() > 0)
        program_uniform(program, "inverse", 1); 
    else
        program_uniform(program, "inverse", 0); 


    program_uniform(program, "vpMatrix", projection * view);
    program_uniform(program, "viewMatrix", view);
    //m_multi_indirect.clear();
    std::vector<int> regions;
    int t =0;
    /*
    if(regionsVisible.size() > 0) {
        for(int i= 0; i<regionsVisible.size(); i++)
                drawRegion(regionsVisible[i],nbVertex);
        regions = regionsVisible;
    } else {*/
    for(int i=0; i< m_nbRegions*m_nbRegions; i++) {
        bool test = false;
        if(t<regions.size() && regions[t] == i) {
            test = true;
            t++;
        }
        if(test || visbleCamera(i,mvp)) {
            //pushRegionsVectorIndirect(i,nbVertex);
            drawRegion(i,nbVertex);
            regions.push_back(i);
        }
    }
    //printf("size : %d %d\n",m_multi_indirect.size(),regions.size());
    //glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_multi_indirect.size(), 0);
    
    return regions;
}

void Terrain::pushRegionsVectorIndirect(int i, int nbVertex) {
    //m_multi_indirect.push_back( { unsigned(nbVertex), m_sizeRegion, i*m_sizeRegion, i*m_sizeRegion} );
}

void Terrain::hauteurCamera(Camera &cam) {
    Point p = cam.position();  
    float h = getHauteur(p.x,-p.z)+5;
    if(p.y < h)
        p.y = std::min(h,p.y + 0.2f);
    else
        p.y = std::max(h,p.y - 0.2f);

    p.y = std::max(h-3,p.y);
    cam.setPosition(p);
}
