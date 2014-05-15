#include "langmuirviewer.h"
#include "color.h"
#include "box.h"

#include <QVector>
#include <QDebug>
#include <QImage>

Box::Box(LangmuirViewer &viewer, QObject *parent) :
    SceneObject(viewer, parent)
{
    init();
}

Box::~Box()
{
    glDeleteTextures(1, &m_imageID);
}

const QColor& Box::getColor() const
{
    return m_color;
}

int Box::getXSize() const
{
    return m_xsize;
}

int Box::getYSize() const
{
    return m_ysize;
}

int Box::getZSize() const
{
    return m_zsize;
}

bool Box::imageIsOn() const
{
    return m_imageOn;
}

void Box::init() {
    m_color     = Qt::red;
    m_xsize     = 1.0;
    m_ysize     = 1.0;
    m_zsize     = 1.0;
    m_halfXSize = 0.5;
    m_halfYSize = 0.5;
    m_halfZSize = 0.5;
    m_imageID   = 0;
    m_imageOn   = false;
    m_faces     = All;

    emit colorChanged(m_color);
    emit xSizeChanged(m_xsize);
    emit ySizeChanged(m_ysize);
    emit zSizeChanged(m_zsize);
}

void Box::draw() {

    GLboolean texture2DIsOn;
    glGetBooleanv(GL_TEXTURE_2D, &texture2DIsOn);

    //bind texture
    glBindTexture(GL_TEXTURE_2D, m_imageID);

    static float color[4];

    if (m_imageOn) {
        glEnable(GL_TEXTURE_2D);
        color::qColorToArray4(Qt::white, color);
    }
    else {
        glDisable(GL_TEXTURE_2D);
        color::qColorToArray4(m_color, color);
    }

    glBegin(GL_QUADS);

    glColor4fv(color);

    // (+Y)
    if (m_faces.testFlag(North)) {
        glTexCoord2f(1.0f, 1.0f); glVertex3f( m_halfXSize, m_halfYSize,-m_halfZSize); glNormal3f( 0.0f, 1.0f, 0.0f); // NE
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-m_halfXSize, m_halfYSize,-m_halfZSize); glNormal3f( 0.0f, 1.0f, 0.0f); // NW
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-m_halfXSize, m_halfYSize, m_halfZSize); glNormal3f( 0.0f, 1.0f, 0.0f); // SW
        glTexCoord2f(1.0f, 0.0f); glVertex3f( m_halfXSize, m_halfYSize, m_halfZSize); glNormal3f( 0.0f, 1.0f, 0.0f); // SE
    }

    // (-Y)
    if (m_faces.testFlag(South)) {
        glTexCoord2f(1.0f, 1.0f); glVertex3f( m_halfXSize,-m_halfYSize, m_halfZSize); glNormal3f( 0.0f,-1.0f, 0.0f); // NE
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-m_halfXSize,-m_halfYSize, m_halfZSize); glNormal3f( 0.0f,-1.0f, 0.0f); // NW
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-m_halfXSize,-m_halfYSize,-m_halfZSize); glNormal3f( 0.0f,-1.0f, 0.0f); // SW
        glTexCoord2f(1.0f, 0.0f); glVertex3f( m_halfXSize,-m_halfYSize,-m_halfZSize); glNormal3f( 0.0f,-1.0f, 0.0f); // SE
    }

    // (+Z)
    if (m_faces.testFlag(Front)) {
        glTexCoord2f(1.0f, 1.0f); glVertex3f( m_halfXSize,-m_halfYSize, m_halfZSize); glNormal3f( 0.0f, 0.0f, 1.0f); // SW
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-m_halfXSize,-m_halfYSize, m_halfZSize); glNormal3f( 0.0f, 0.0f, 1.0f); // SE
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-m_halfXSize, m_halfYSize, m_halfZSize); glNormal3f( 0.0f, 0.0f, 1.0f); // NE
        glTexCoord2f(1.0f, 0.0f); glVertex3f( m_halfXSize, m_halfYSize, m_halfZSize); glNormal3f( 0.0f, 0.0f, 1.0f); // NW
    }

    // (-Z)
    if (m_faces.testFlag(Back)) {
        glTexCoord2f(1.0f, 1.0f); glVertex3f( m_halfXSize,-m_halfYSize,-m_halfZSize); glNormal3f( 0.0f, 0.0f,-1.0f); // SW
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-m_halfXSize,-m_halfYSize,-m_halfZSize); glNormal3f( 0.0f, 0.0f,-1.0f); // SE
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-m_halfXSize, m_halfYSize,-m_halfZSize); glNormal3f( 0.0f, 0.0f,-1.0f); // NE
        glTexCoord2f(1.0f, 0.0f); glVertex3f( m_halfXSize, m_halfYSize,-m_halfZSize); glNormal3f( 0.0f, 0.0f,-1.0f); // NW
    }

    // (+X)
    if (m_faces.testFlag(East)) {
        glTexCoord2f(1.0f, 1.0f); glVertex3f( m_halfXSize, m_halfYSize,-m_halfZSize); glNormal3f( 1.0f, 0.0f, 0.0f); // NE
        glTexCoord2f(0.0f, 1.0f); glVertex3f( m_halfXSize, m_halfYSize, m_halfZSize); glNormal3f( 1.0f, 0.0f, 0.0f); // NW
        glTexCoord2f(0.0f, 0.0f); glVertex3f( m_halfXSize,-m_halfYSize, m_halfZSize); glNormal3f( 1.0f, 0.0f, 0.0f); // SW
        glTexCoord2f(1.0f, 0.0f); glVertex3f( m_halfXSize,-m_halfYSize,-m_halfZSize); glNormal3f( 1.0f, 0.0f, 0.0f); // SE
    }

    // (-X)
    if (m_faces.testFlag(West)) {
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-m_halfXSize, m_halfYSize, m_halfZSize); glNormal3f(-1.0f, 0.0f, 0.0f); // NE
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-m_halfXSize, m_halfYSize,-m_halfZSize); glNormal3f(-1.0f, 0.0f, 0.0f); // NW
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-m_halfXSize,-m_halfYSize,-m_halfZSize); glNormal3f(-1.0f, 0.0f, 0.0f); // SW
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-m_halfXSize,-m_halfYSize, m_halfZSize); glNormal3f(-1.0f, 0.0f, 0.0f); // SE
    }

    glEnd();

    if (texture2DIsOn) {
        glEnable(GL_TEXTURE_2D);
    }
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
        m_halfXSize = 0.5 * m_xsize;
        emit xSizeChanged(value);
    }
}

void Box::setYSize(double value)
{
    if (value != m_ysize) {
        m_ysize = value;
        m_halfYSize = 0.5 * m_ysize;
        emit ySizeChanged(value);
    }
}

void Box::setZSize(double value)
{
    if (value != m_zsize) {
        m_zsize = value;
        m_halfZSize = 0.5 * m_zsize;
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
    connect(this, SIGNAL(imageOnChanged(bool)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(facesChanged(Faces)) , &m_viewer, SLOT(updateGL()));
}

void Box::setFaces(Faces faces)
{
    m_faces = faces;
}

void Box::loadImage(const QImage &image)
{
    QImage i = m_viewer.convertToGLFormat(image);
    glDeleteTextures(1, &m_imageID);
    glGenTextures(1, &m_imageID);
    glBindTexture(GL_TEXTURE_2D, m_imageID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, i.width(), i.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, i.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Box::showImage(bool on)
{
    m_imageOn = on;
    emit imageOnChanged(m_imageOn);
}

void Box::toggleImage()
{
    m_imageOn = !m_imageOn;
    emit imageOnChanged(m_imageOn);
}
