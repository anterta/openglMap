#ifndef DEF_CAMERA
#define DEF_CAMERA

#include "gKit/mat.h"

class Camera
{
    public:

    Camera() :  m_phi(0.0), m_theta(0.0), m_orientation(1,0,0), m_deplacementLateral(0,1,0), m_axeVertical(0, 0, 1),
                    m_axeLateral(0,1,0), m_position(), m_sensibilite(0.0), m_vitesse(0.0) { }


    Camera(Vector position, Vector axeVertical, Vector axeLateral, double sensibilite, double vitesse) : 
            m_phi(0.0), m_theta(0.0), m_orientation(cross(axeLateral,axeVertical)), m_deplacementLateral(axeLateral), m_axeVertical(axeVertical),
            m_axeLateral(axeLateral), m_position(position), m_sensibilite(sensibilite), m_vitesse(vitesse) { }

    ~Camera() { }

    void orienter(int xRel, int yRel);
    void deplacer();
    Transform lookAt(Vector t = Vector(0,0,0), bool inverse = false);

    void setSensibilite(double sensibilite) { m_sensibilite = sensibilite; }
    void setVitesse(double vitesse) { m_vitesse = vitesse; }
    void setPointcible(Point pointCible);
    void setPosition(Point position) { m_position = position; }

    double getSensibilite() const { return m_sensibilite; }
    double getVitesse() const { return m_vitesse; }
    Point  getPosition() { return m_position; }
    Vector getOrientation() {return m_orientation; }
    Vector getAxeLateral() { return m_deplacementLateral; }
    double getPhi() { return m_phi/90; }


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
