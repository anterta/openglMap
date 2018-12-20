#include "terrain.h"
#include "time.h"

void Terrain::make_terrain( const char *filename, const int nbRegions) {
    m_cube = read_mesh("data/cube.obj");;
    m_multi_draw.initialiser(m_cube);
    m_nbRegions = nbRegions;

    Image data = read_image(filename);
            
    const float nbCubes = (float)nbRegions*64.0;

    //float height[(int)nbCubes*(int)nbCubes];
    // genere les positions des sommets sur la grille
    float coord_scaleY = float(data.height())/nbCubes;
    float coord_scaleX = float(data.width())/nbCubes;

    int random = rand() % 1000 + 100;
    int count = 0;
    float heightWater = nbRegions * 0.2;
    float heightSnow = nbRegions * 2.9;

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
                    Vector tmp(x, a, -y);
                    m_mesh.vertex(x, a, -y);
                    //height[indice(x,y)] = a;
                    m_multi_draw.addPos(tmp);
                    if(a>heightWater && a<heightSnow) {
                        random--;
                        if(random == 0) {
                            count++;
                            //std::cout << count << std::endl;
                            m_billboards.addPos(tmp+Vector(0,5,0));
                            //random = rand() % 1000 + 100;
                            random = rand() % 100 + 10;
                        }
                    }
                }
            m_min_max.push_back(vec2(hmin,hmax));
        }
    m_multi_draw.createBuffer("src/shader_indirect_cull.glsl"); 
    m_mesh.release();
}

void Terrain::bindBuffers( const Transform& mvp ) {
    m_multi_draw.bindBuffers(mvp);
}

void Terrain::multiDraw(const GLuint& program, const Transform& p, const Transform& v) {
    program_uniform(program, "vpMatrix", p*v);
    program_uniform(program, "nbCubes", m_nbRegions*64);
    program_uniform(program, "inverseMatrix", v.inverse());

    m_multi_draw.multiDraw();
    m_billboards.show(p*v);
}

void Terrain::multiDraw() {
    m_multi_draw.multiDraw();
}

void Terrain::release() {
    m_multi_draw.release();
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
