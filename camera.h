#ifndef DEF_CAMERA
#define DEF_CAMERA

#include "mat.h"
#include "vec.h"

class Camera
{
    public:

    Camera();
    Camera(Vector position, Vector axeVertical, Vector axeLateral, double sensibilite, double vitesse);
    ~Camera();

    void orienter(int xRel, int yRel);
    void deplacer();
    Transform lookAt(Vector t = Vector(0,0,0), bool inverse = false);

    void setPointcible(Point pointCible);
    void setPosition(Point position);

    double getSensibilite() const;
    double getVitesse() const;

    void setSensibilite(double sensibilite);
    void setVitesse(double vitesse);

    Point position(); // dans le repère Monde
    Vector orientation() {return m_orientation; }
    Vector lateral() { return m_deplacementLateral; }


    private:

    double m_phi;
    double m_theta;
    Vector m_orientation;
    Vector m_deplacementLateral;

    Vector m_axeVertical;
    Vector m_axeLateral;

    Point m_position;

    double m_sensibilite;
    double m_vitesse;
};

#endif
