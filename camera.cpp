#include "camera.h"
#include <math.h>
#include <SDL2/SDL.h>


// Constructeurs et Destructeur

Camera::Camera() :  m_phi(0.0), m_theta(0.0), m_orientation(1,0,0), m_deplacementLateral(0,1,0), m_axeVertical(0, 0, 1),
                    m_axeLateral(0,1,0), m_position(), m_sensibilite(0.0), m_vitesse(0.0) { }


Camera::Camera(Vector position, Vector axeVertical, Vector axeLateral, double sensibilite, double vitesse) : 
            m_phi(0.0), m_theta(0.0), m_orientation(cross(axeLateral,axeVertical)), m_deplacementLateral(axeLateral), m_axeVertical(axeVertical),
            m_axeLateral(axeLateral), m_position(position), m_sensibilite(sensibilite), m_vitesse(vitesse)
{ }


Camera::~Camera() { }


// M�thodes

void Camera::orienter(int xRel, int yRel)
{
    double tmp_theta = m_theta;
    // R�cup�ration des angles
    m_phi += -yRel * m_sensibilite;
    m_theta += -xRel * m_sensibilite;


    // Limitation de l'angle phi
    if(m_phi > 89.0)
        m_phi = 89.0;

    else if(m_phi < -89.0)
        m_phi = -89.0;

    Transform tmp = Rotation(m_axeVertical, tmp_theta-m_theta);
    m_orientation = tmp(m_orientation);
    m_deplacementLateral = tmp(m_deplacementLateral);
}

void Camera::deplacer(Terrain terrain)
{
    // Orientation avec la souris
    int mx, my;
    unsigned int mb= SDL_GetRelativeMouseState(&mx, &my);
    if(mb & SDL_BUTTON(1))
        orienter(mx,my);


    // Déplacement au clavier
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    if(state[SDL_SCANCODE_UP])
        m_position = m_position - m_orientation * m_vitesse;

    if(state[SDL_SCANCODE_DOWN])
        m_position = m_position + m_orientation * m_vitesse;

    if(state[SDL_SCANCODE_LEFT])
        m_position = m_position - m_deplacementLateral * m_vitesse;

    if(state[SDL_SCANCODE_RIGHT])
        m_position = m_position + m_deplacementLateral * m_vitesse;

    m_position.y = terrain.getHauteur(m_position.x,-m_position.z);
    m_position.y+=3;
}

Transform Camera::lookAt()
{
    return Rotation(m_axeLateral, m_phi) * Rotation(m_axeVertical, m_theta) * Translation( -m_position.x, -m_position.y, -m_position.z );
}


// Getters et Setters
void Camera::setPointcible(Vector pointCible)
{
    // Calcul du vecteur orientation
    m_orientation = m_position - pointCible;
    m_orientation = normalize(m_orientation);

    // Si l'axe vertical est l'axe X
    if(m_axeVertical.x == 1.0)
    {
        // Calcul des angles
        m_phi = asin(m_orientation.x);
        m_theta = acos(m_orientation.y / cos(m_phi));

        if(m_orientation.y < 0)
            m_theta *= -1;
    }

    // Si c'est l'axe Y
    else if(m_axeVertical.y == 1.0)
    {
        // Calcul des angles
        m_phi = asin(m_orientation.y);
        m_theta = acos(m_orientation.z / cos(m_phi));

        if(m_orientation.z < 0)
            m_theta *= -1;
    }

    // Sinon c'est l'axe Z
    else
    {
        // Calcul des angles
        m_phi = asin(m_orientation.x);
        m_theta = acos(m_orientation.z / cos(m_phi));

        if(m_orientation.z < 0)
            m_theta *= -1;
    }

    m_deplacementLateral = cross(m_orientation,m_axeVertical);

    // Conversion en degr�s
    m_phi = m_phi * 180 / M_PI;
    m_theta = m_theta * 180 / M_PI;
}


void Camera::setPosition(Vector position)
{
    m_position = position;
}


double Camera::getSensibilite() const
{
    return m_vitesse;
}


double Camera::getVitesse() const
{
    return m_vitesse;
}


void Camera::setSensibilite(double sensibilite)
{
    m_sensibilite = sensibilite;
}


void Camera::setVitesse(double vitesse)
{
    m_vitesse = vitesse;
}

Point Camera::position( )
{
    Transform t= lookAt();     // passage monde vers camera
    Transform tinv= t.inverse();            // l'inverse, passage camera vers monde
    
    return tinv(Point(0, 0, 0));        // la camera se trouve a l'origine, dans le repere camera...
}
