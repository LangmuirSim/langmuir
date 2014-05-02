#include "color.h"

#ifdef Q_OS_MAC
    #include <OpenGL/glu.h>
#else
    #include <GL/glu.h>
#endif

void applyColor(const QColor& color, const QColor &se_color)
{
    static float m_float4[4];

    // set specular & emission
    qColorToArray4(se_color, m_float4);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, m_float4);

    // set ambient and diffuse to white
    qColorToArray4(color, m_float4);
    glColor4f(m_float4[0], m_float4[1], m_float4[2], m_float4[3]);
}
