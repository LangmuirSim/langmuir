#include "corneraxis.h"
#include "langmuirviewer.h"

#include <QDebug>

CornerAxis::CornerAxis(LangmuirViewer &viewer, QObject *parent) :
    Axis(viewer, parent)
{
    init();
}

CornerAxis::Location CornerAxis::getLocation() const
{
    return m_location;
}

int CornerAxis::getSize() const
{
    return m_size;
}

int CornerAxis::getShift() const
{
    return m_shift;
}

void CornerAxis::init()
{
    m_location = LowerLeft;
    m_shift = 10;
    m_size = 150;

    emit locationChanged(m_location);
    emit sizeChanged(m_size);
}

void CornerAxis::preDraw()
{
    // original viewport saved
    glGetIntegerv(GL_VIEWPORT, m_viewPort);
    glGetIntegerv(GL_SCISSOR_BOX, m_scissorBox);

    // view port is moved
    int viewPort[4];
    switch (m_location)
    {
        case UpperRight:
        {
            viewPort[0] = m_viewPort[0] + m_viewPort[2] - m_shift - m_size;
            viewPort[1] = m_viewPort[1] + m_viewPort[3] - m_shift - m_size;
            viewPort[2] = m_size;
            viewPort[3] = m_size;
            break;
        }
        case LowerRight:
        {
            viewPort[0] = m_viewPort[0] + m_viewPort[2] -m_shift - m_size;
            viewPort[1] = m_shift;
            viewPort[2] = m_size;
            viewPort[3] = m_size;
            break;
        }
        case UpperLeft:
        {
            viewPort[0] = m_shift;
            viewPort[1] = m_viewPort[1] + m_viewPort[3] - m_shift - m_size;
            viewPort[2] = m_size;
            viewPort[3] = m_size;
            break;
        }
        case LowerLeft: default:
        {
            viewPort[0] = m_shift;
            viewPort[1] = m_shift;
            viewPort[2] = m_size;
            viewPort[3] = m_size;
            break;
        }
    }
    glViewport(viewPort[0], viewPort[1], viewPort[2], viewPort[3]);
    glScissor(0, 0, m_size, m_size);

    // axis appears on top of zbuffer
    glClear(GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMultMatrixd(m_viewer.camera()->orientation().inverse().matrix());
}

void CornerAxis::postDraw()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_LIGHTING);

    glScissor(m_scissorBox[0], m_scissorBox[1], m_scissorBox[2], m_scissorBox[3]);
    glViewport(m_viewPort[0], m_viewPort[1], m_viewPort[2], m_viewPort[3]);
}

void CornerAxis::setLocation(Location location)
{
    if (location != m_location) {
        m_location = location;
        emit locationChanged(m_location);
    }
}

void CornerAxis::setShift(int shift)
{
    if (shift != m_shift) {
        m_shift = shift;
        emit shiftChanged(m_shift);
    }
}

void CornerAxis::setSize(int size)
{
    if (size != m_size) {
        m_size = size;
        emit sizeChanged(m_size);
    }
}

void CornerAxis::makeConnections()
{
    Axis::makeConnections();
    connect(this, SIGNAL(locationChanged(Location)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(shiftChanged(int)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(sizeChanged(int)), &m_viewer, SLOT(updateGL()));
}
