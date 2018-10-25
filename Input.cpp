#include "Input.h"


// Constructeur et Destructeur

Input::Input() : m_x(0), m_y(0), m_xRel(0), m_yRel(0), m_terminer(false)
{
    // Initialisation du tableau m_touches[]

    for(int i(0); i < SDL_NUM_SCANCODES; i++)
        m_touches[i] = false;


    // Initialisation du tableau m_boutonsSouris[]

    for(int i(0); i < 8; i++)
        m_boutonsSouris[i] = false;
}


Input::~Input()
{

}


// Méthodes

void Input::updateEvenements()
{
    // Pour éviter des mouvements fictifs de la souris, on réinitialise les coordonnées relatives

    m_xRel = 0;
    m_yRel = 0;


    // Boucle d'évènements

    while(SDL_PollEvent(&m_evenements))
    {
        // Switch sur le type d'évènement

        switch(m_evenements.type)
        {
            // Cas d'une touche enfoncée

            case SDL_KEYDOWN:
                m_touches[m_evenements.key.keysym.scancode] = true;
            break;


            // Cas d'une touche relâchée

            case SDL_KEYUP:
                m_touches[m_evenements.key.keysym.scancode] = false;
            break;


            // Cas de pression sur un bouton de la souris

            case SDL_MOUSEBUTTONDOWN:

                m_boutonsSouris[m_evenements.button.button] = true;

            break;


            // Cas du relâchement d'un bouton de la souris

            case SDL_MOUSEBUTTONUP:

                m_boutonsSouris[m_evenements.button.button] = false;

            break;


            // Cas d'un mouvement de souris

            case SDL_MOUSEMOTION:

                m_x = m_evenements.motion.x;
                m_y = m_evenements.motion.y;

                m_xRel = m_evenements.motion.xrel;
                m_yRel = m_evenements.motion.yrel;

            break;


            // Cas de la fermeture de la fenêtre

            case SDL_WINDOWEVENT:

                if(m_evenements.window.event == SDL_WINDOWEVENT_CLOSE)
                    m_terminer = true;

            break;


            default:
            break;
        }
    }
}


bool Input::terminer() const
{
    return m_terminer;
}


void Input::afficherPointeur(bool reponse) const
{
    if(reponse)
        SDL_ShowCursor(SDL_ENABLE);

    else
        SDL_ShowCursor(SDL_DISABLE);
}


void Input::capturerPointeur(bool reponse) const
{
    if(reponse)
        SDL_SetRelativeMouseMode(SDL_TRUE);

    else
        SDL_SetRelativeMouseMode(SDL_FALSE);
}



// Getters

bool Input::getTouche(const SDL_Scancode touche) const
{
    return m_touches[touche];
}


bool Input::getBoutonSouris(const Uint8 bouton) const
{
    return m_boutonsSouris[bouton];
}


bool Input::mouvementSouris() const
{
    if(m_xRel == 0 && m_yRel == 0)
        return false;

    else
        return true;
}


// Getters concernant la position du curseur

int Input::getX() const
{
    return m_x;
}

int Input::getY() const
{
    return m_y;
}

int Input::getXRel() const
{
    return m_xRel;
}

int Input::getYRel() const
{
    return m_yRel;
}
