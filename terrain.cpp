#include "terrain.h"

void Terrain::make_terrain( const char *filename, const int nbRegions ) {
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
                    float a= data.sample(x*coord_scaleX, y*coord_scaleY).r * nbCubes/15.0;
                    
                    if(a>hmax)
                        hmax=a;
                    if(a<hmin)
                        hmin=a;

                    // genere la position
                    m_mesh.vertex(x, a, -y);
                    height[indice(x,y)] = a;
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
                    if(x<nbRegions*64)
                        hR = height[indice(x+1,y)];
                    if(y>0)
                        hD = height[indice(x,y-1)];
                    if(y<nbRegions*64)
                        hU = height[indice(x,y+1)];

                    m_mesh.normal(normalize(Vector(hL - hR,2.0,hD - hU)));
                }

}

void Terrain::createBuffer(GLuint &buffer) {
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    
    size_t size= m_mesh.vertex_buffer_size() + m_mesh.normal_buffer_size();
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
    
    // configure l'attribut 1, vec3 position
    size= m_mesh.vertex_buffer_size();
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, m_mesh.vertex_buffer());
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(1);

    // configure l'attribut 2, vec3 normal
    size_t offset= size;
    size= m_mesh.normal_buffer_size();
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, m_mesh.normal_buffer());
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *) offset);
    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(2);
    
    m_mesh.release();
}

bool visible( const Transform& mvp, const Point& pmin, const Point& pmax )
{
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
    return visible(mvp,pmin,pmax);

    vec4 vmin = mvp(vec4(pmin));
    vec4 vmax = mvp(vec4(pmax));

    vec4 vmin2 = mvp(vec4(Point(x*64,m_min_max[i].x,-(y+1)*64)));
    vec4 vmax2 = mvp(vec4(Point((x+1)*64,m_min_max[i].y,-y*64)));
    
    return (((vmin.x > -vmin.w || vmax.x > -vmax.w) && (vmin.x < vmin.w || vmax.x < vmax.w)) && 
            ((vmin.y > -vmin.w || vmax.y > -vmax.w) && (vmin.y < vmin.w || vmax.y < vmax.w)) && 
            ((vmin.z > -vmin.w || vmax.z > -vmax.w) && (vmin.z < vmin.w || vmax.z < vmax.w)))
        || (((vmin2.x > -vmin2.w || vmax2.x > -vmax2.w) && (vmin2.x < vmin2.w || vmax2.x < vmax2.w)) && 
            ((vmin2.y > -vmin2.w || vmax2.y > -vmax2.w) && (vmin2.y < vmin2.w || vmax2.y < vmax2.w)) && 
            ((vmin2.z > -vmin2.w || vmax2.z > -vmax2.w) && (vmin2.z < vmin2.w || vmax2.z < vmax2.w)));
}
