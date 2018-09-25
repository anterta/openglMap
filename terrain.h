
#ifndef _TERRAIN_H
#define _TERRAIN_H

#include "mat.h"
#include "mesh.h"
#include "image.h"
#include "image_io.h"

class Terrain
{
    public:
    Terrain() {};
    
    void make_terrain( const char *filename, const int nbRegions ) {
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

    int indice(int x,int y) {
        return y * m_nbRegions*64.0 + x;
    }

    void createBuffer(GLuint &buffer) {
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

    bool visbleCamera(int i, Transform mvp) {
        int y = i/m_nbRegions;
        int x = i - y * m_nbRegions;
        vec4 vmin = mvp(vec4(Point(x*64,m_min_max[i].x,-y*64)));
        vec4 vmax = mvp(vec4(Point((x+1)*64,m_min_max[i].y,-(y+1)*64)));

        vec4 vmin2 = mvp(vec4(Point(x*64,m_min_max[i].x,-(y+1)*64)));
        vec4 vmax2 = mvp(vec4(Point((x+1)*64,m_min_max[i].y,-y*64)));
        
        return (((vmin.x > -vmin.w || vmax.x > -vmax.w) && (vmin.x < vmin.w || vmax.x < vmax.w)) && 
                ((vmin.y > -vmin.w || vmax.y > -vmax.w) && (vmin.y < vmin.w || vmax.y < vmax.w)) && 
                ((vmin.z > -vmin.w || vmax.z > -vmax.w) && (vmin.z < vmin.w || vmax.z < vmax.w)))
            || (((vmin2.x > -vmin2.w || vmax2.x > -vmax2.w) && (vmin2.x < vmin2.w || vmax2.x < vmax2.w)) && 
                ((vmin2.y > -vmin2.w || vmax2.y > -vmax2.w) && (vmin2.y < vmin2.w || vmax2.y < vmax2.w)) && 
                ((vmin2.z > -vmin2.w || vmax2.z > -vmax2.w) && (vmin2.z < vmin2.w || vmax2.z < vmax2.w)));
    }

    void drawRegion(int i, int vertex_count) {
        glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, vertex_count, m_sizeRegion, i*m_sizeRegion);
    }

    int sizeRegion() { return m_sizeRegion; }
    Mesh mesh() { return m_mesh; }
    void bounds(Point &pmin, Point &pmax) { m_mesh.bounds(pmin,pmax); }
    int nbRegions() { return m_nbRegions; }


    protected:
    Mesh m_mesh;
    int m_nbRegions;
    int m_sizeRegion = 64*64;
    std::vector<vec2> m_min_max;
};

#endif
