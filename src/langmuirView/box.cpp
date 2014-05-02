#include "langmuirviewer.h"
#include "color.h"
#include "box.h"

#include <QOpenGLBuffer>
#include <QVector>
#include <QDebug>

#ifdef Q_OS_MAC
    #include <OpenGL/glu.h>
#else
    #include <GL/glu.h>
#endif

Box::Box(LangmuirViewer &viewer, QObject *parent) :
    SceneObject(viewer, parent)
{
    init();
}

void Box::init() {
    m_color = QColor::fromRgbF(1.0, 0.0, 0.0, 0.0);
    emit colorChanged(m_color);
    m_xsize = 1.0;
    m_ysize = 1.0;
    m_zsize = 1.0;
}

void Box::draw() {
    glBegin(GL_QUADS);

    // (+Y)
    glColor3f(0.0f,1.0f,0.0f);
    glNormal3f( 0.0f, 1.0f, 0.0f);
    glVertex3f( m_xsize, m_ysize,-m_zsize); // NE
    glVertex3f(-m_xsize, m_ysize,-m_zsize); // NW
    glVertex3f(-m_xsize, m_ysize, m_zsize); // SW
    glVertex3f( m_xsize, m_ysize, m_zsize); // SE

    // (-Y)
    glColor3f(1.0f,0.5f,0.0f);
    glNormal3f( 0.0f,-1.0f, 0.0f);
    glVertex3f( m_xsize,-m_ysize, m_zsize); // NE
    glVertex3f(-m_xsize,-m_ysize, m_zsize); // NW
    glVertex3f(-m_xsize,-m_ysize,-m_zsize); // SW
    glVertex3f( m_xsize,-m_ysize,-m_zsize); // SE

    // (+Z)
    glColor3f(1.0f,0.0f,0.0f);
    glNormal3f( 0.0f, 0.0f, 1.0f);
    glVertex3f( m_xsize, m_ysize, m_zsize); // NE
    glVertex3f(-m_xsize, m_ysize, m_zsize); // NW
    glVertex3f(-m_xsize,-m_ysize, m_zsize); // SW
    glVertex3f( m_xsize,-m_ysize, m_zsize); // SE

    // (-Z)
    glColor3f(1.0f,1.0f,0.0f);
    glNormal3f( 0.0f, 0.0f,-1.0f);
    glVertex3f( m_xsize,-m_ysize,-m_zsize); // SW
    glVertex3f(-m_xsize,-m_ysize,-m_zsize); // SE
    glVertex3f(-m_xsize, m_ysize,-m_zsize); // NE
    glVertex3f( m_xsize, m_ysize,-m_zsize); // NW

    // (+X)
    glColor3f(1.0f,0.0f,1.0f);
    glNormal3f( 1.0f, 0.0f, 0.0f);
    glVertex3f( m_xsize, m_ysize,-m_zsize); // NE
    glVertex3f( m_xsize, m_ysize, m_zsize); // NW
    glVertex3f( m_xsize,-m_ysize, m_zsize); // SW
    glVertex3f( m_xsize,-m_ysize,-m_zsize); // SE

    // (-X)
    glColor3f(0.0f,0.0f,1.0f);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-m_xsize, m_ysize, m_zsize); // NE
    glVertex3f(-m_xsize, m_ysize,-m_zsize); // NW
    glVertex3f(-m_xsize,-m_ysize,-m_zsize); // SW
    glVertex3f(-m_xsize,-m_ysize, m_zsize); // SE

    glEnd();
}

void Box::setColor(QColor color)
{
    if (color != m_color) {
        m_color = color;
        emit colorChanged(m_color);
    }
}

void Box::setXSize(double value)
{
    if (value != m_xsize) {
        m_xsize = value;
        emit xSizeChanged(value);
    }
}

void Box::setYSize(double value)
{
    if (value != m_ysize) {
        m_ysize = value;
        emit ySizeChanged(value);
    }
}

void Box::setZSize(double value)
{
    if (value != m_zsize) {
        m_zsize = value;
        emit zSizeChanged(value);
    }
}

void Box::makeConnections()
{
    SceneObject::makeConnections();
    connect(this, SIGNAL(colorChanged(QColor)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(xSizeChanged(double)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(ySizeChanged(double)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(zSizeChanged(double)), &m_viewer, SLOT(updateGL()));
}
