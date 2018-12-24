
#include "camera.h"
#include <math.h>
#include <SDL2/SDL.h>


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

void Camera::deplacer()
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
}

Transform Camera::lookAt(Vector t, bool inverse)
{
    if(inverse)
        return Rotation(m_axeLateral, -m_phi) * Rotation(m_axeVertical, m_theta) * Translation( -m_position.x+t.x, -m_position.y+t.y, -m_position.z+t.z );

    return Rotation(m_axeLateral, m_phi) * Rotation(m_axeVertical, m_theta) * Translation( -m_position.x+t.x, -m_position.y+t.y, -m_position.z+t.z );;
}

// Getters et Setters
void Camera::setPointcible(Point pointCible)
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
