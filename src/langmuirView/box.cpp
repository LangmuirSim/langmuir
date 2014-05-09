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

    emit colorChanged(m_color);
    emit xSizeChanged(m_xsize);
    emit ySizeChanged(m_ysize);
    emit zSizeChanged(m_zsize);
}

void Box::draw() {

    GLboolean texture2DOn;
    glGetBooleanv(GL_TEXTURE_2D, &texture2DOn);

    GLboolean colorMaterialOn;
    glGetBooleanv(GL_COLOR_MATERIAL, &colorMaterialOn);

    glBegin(GL_QUADS);

    //bind texture
    glBindTexture(GL_TEXTURE_2D, m_imageID);

    static float white[4];
    static float color[4];

    color::qColorToArray4(Qt::white, white);
    color::qColorToArray4(m_color, color);

    // (+Y)
    if (m_imageOn && m_faces.testFlag(North)) {
        glColor4fv(white);
        glEnable(GL_TEXTURE_2D);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( m_halfXSize, m_halfYSize,-m_halfZSize); glNormal3f( 0.0f, 1.0f, 0.0f); // NE
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-m_halfXSize, m_halfYSize,-m_halfZSize); glNormal3f( 0.0f, 1.0f, 0.0f); // NW
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-m_halfXSize, m_halfYSize, m_halfZSize); glNormal3f( 0.0f, 1.0f, 0.0f); // SW
            glTexCoord2f(1.0f, 0.0f); glVertex3f( m_halfXSize, m_halfYSize, m_halfZSize); glNormal3f( 0.0f, 1.0f, 0.0f); // SE
        glDisable(GL_TEXTURE_2D);
    } else {
        glColor4fv(color);
        glVertex3f( m_halfXSize, m_halfYSize,-m_halfZSize); glNormal3f( 0.0f, 1.0f, 0.0f); // NE
        glVertex3f(-m_halfXSize, m_halfYSize,-m_halfZSize); glNormal3f( 0.0f, 1.0f, 0.0f); // NW
        glVertex3f(-m_halfXSize, m_halfYSize, m_halfZSize); glNormal3f( 0.0f, 1.0f, 0.0f); // SW
        glVertex3f( m_halfXSize, m_halfYSize, m_halfZSize); glNormal3f( 0.0f, 1.0f, 0.0f); // SE
    }

    // (-Y)
    if (m_imageOn && m_faces.testFlag(South)) {
        glColor4fv(white);
        glEnable(GL_TEXTURE_2D);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( m_halfXSize,-m_halfYSize, m_halfZSize); glNormal3f( 0.0f,-1.0f, 0.0f); // NE
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-m_halfXSize,-m_halfYSize, m_halfZSize); glNormal3f( 0.0f,-1.0f, 0.0f); // NW
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-m_halfXSize,-m_halfYSize,-m_halfZSize); glNormal3f( 0.0f,-1.0f, 0.0f); // SW
            glTexCoord2f(1.0f, 0.0f); glVertex3f( m_halfXSize,-m_halfYSize,-m_halfZSize); glNormal3f( 0.0f,-1.0f, 0.0f); // SE
        glDisable(GL_TEXTURE_2D);
    } else {
        glColor4fv(color);
        glVertex3f( m_halfXSize,-m_halfYSize, m_halfZSize); glNormal3f( 0.0f,-1.0f, 0.0f); // NE
        glVertex3f(-m_halfXSize,-m_halfYSize, m_halfZSize); glNormal3f( 0.0f,-1.0f, 0.0f); // NW
        glVertex3f(-m_halfXSize,-m_halfYSize,-m_halfZSize); glNormal3f( 0.0f,-1.0f, 0.0f); // SW
        glVertex3f( m_halfXSize,-m_halfYSize,-m_halfZSize); glNormal3f( 0.0f,-1.0f, 0.0f); // SE
    }

    // (+Z)
    if (m_imageOn && m_faces.testFlag(Front)) {
        glColor4fv(white);
        glEnable(GL_TEXTURE_2D);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( m_halfXSize,-m_halfYSize, m_halfZSize); glNormal3f( 0.0f, 0.0f, 1.0f); // SW
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-m_halfXSize,-m_halfYSize, m_halfZSize); glNormal3f( 0.0f, 0.0f, 1.0f); // SE
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-m_halfXSize, m_halfYSize, m_halfZSize); glNormal3f( 0.0f, 0.0f, 1.0f); // NE
            glTexCoord2f(1.0f, 0.0f); glVertex3f( m_halfXSize, m_halfYSize, m_halfZSize); glNormal3f( 0.0f, 0.0f, 1.0f); // NW
        glDisable(GL_TEXTURE_2D);
    } else {
        glColor4fv(color);
        glVertex3f( m_halfXSize,-m_halfYSize, m_halfZSize); glNormal3f( 0.0f, 0.0f, 1.0f); // SW
        glVertex3f(-m_halfXSize,-m_halfYSize, m_halfZSize); glNormal3f( 0.0f, 0.0f, 1.0f); // SE
        glVertex3f(-m_halfXSize, m_halfYSize, m_halfZSize); glNormal3f( 0.0f, 0.0f, 1.0f); // NE
        glVertex3f( m_halfXSize, m_halfYSize, m_halfZSize); glNormal3f( 0.0f, 0.0f, 1.0f); // NW
    }

    // (-Z)
    if (m_imageOn && m_faces.testFlag(Back)) {
        glColor4fv(white);
        glEnable(GL_TEXTURE_2D);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( m_halfXSize,-m_halfYSize,-m_halfZSize); glNormal3f( 0.0f, 0.0f,-1.0f); // SW
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-m_halfXSize,-m_halfYSize,-m_halfZSize); glNormal3f( 0.0f, 0.0f,-1.0f); // SE
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-m_halfXSize, m_halfYSize,-m_halfZSize); glNormal3f( 0.0f, 0.0f,-1.0f); // NE
            glTexCoord2f(1.0f, 0.0f); glVertex3f( m_halfXSize, m_halfYSize,-m_halfZSize); glNormal3f( 0.0f, 0.0f,-1.0f); // NW
        glDisable(GL_TEXTURE_2D);
    } else {
        glColor4fv(color);
        glVertex3f( m_halfXSize,-m_halfYSize,-m_halfZSize); glNormal3f( 0.0f, 0.0f,-1.0f); // SW
        glVertex3f(-m_halfXSize,-m_halfYSize,-m_halfZSize); glNormal3f( 0.0f, 0.0f,-1.0f); // SE
        glVertex3f(-m_halfXSize, m_halfYSize,-m_halfZSize); glNormal3f( 0.0f, 0.0f,-1.0f); // NE
        glVertex3f( m_halfXSize, m_halfYSize,-m_halfZSize); glNormal3f( 0.0f, 0.0f,-1.0f); // NW
    }

    // (+X)
    if (m_imageOn && m_faces.testFlag(East)) {
        glColor4fv(white);
        glEnable(GL_TEXTURE_2D);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( m_halfXSize, m_halfYSize,-m_halfZSize); glNormal3f( 1.0f, 0.0f, 0.0f); // NE
            glTexCoord2f(0.0f, 1.0f); glVertex3f( m_halfXSize, m_halfYSize, m_halfZSize); glNormal3f( 1.0f, 0.0f, 0.0f); // NW
            glTexCoord2f(0.0f, 0.0f); glVertex3f( m_halfXSize,-m_halfYSize, m_halfZSize); glNormal3f( 1.0f, 0.0f, 0.0f); // SW
            glTexCoord2f(1.0f, 0.0f); glVertex3f( m_halfXSize,-m_halfYSize,-m_halfZSize); glNormal3f( 1.0f, 0.0f, 0.0f); // SE
        glDisable(GL_TEXTURE_2D);
    } else {
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
        glColor4fv(color);
        glVertex3f( m_halfXSize, m_halfYSize,-m_halfZSize); glNormal3f( 1.0f, 0.0f, 0.0f); // NE
        glVertex3f( m_halfXSize, m_halfYSize, m_halfZSize); glNormal3f( 1.0f, 0.0f, 0.0f); // NW
        glVertex3f( m_halfXSize,-m_halfYSize, m_halfZSize); glNormal3f( 1.0f, 0.0f, 0.0f); // SW
        glVertex3f( m_halfXSize,-m_halfYSize,-m_halfZSize); glNormal3f( 1.0f, 0.0f, 0.0f); // SE
    }

    // (-X)
    if (m_imageOn && m_faces.testFlag(West)) {
        glColor4fv(white);
        glEnable(GL_TEXTURE_2D);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-m_halfXSize, m_halfYSize, m_halfZSize); glNormal3f(-1.0f, 0.0f, 0.0f); // NE
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-m_halfXSize, m_halfYSize,-m_halfZSize); glNormal3f(-1.0f, 0.0f, 0.0f); // NW
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-m_halfXSize,-m_halfYSize,-m_halfZSize); glNormal3f(-1.0f, 0.0f, 0.0f); // SW
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-m_halfXSize,-m_halfYSize, m_halfZSize); glNormal3f(-1.0f, 0.0f, 0.0f); // SE
        glDisable(GL_TEXTURE_2D);
    } else {
        glColor4fv(color);
        glVertex3f(-m_halfXSize, m_halfYSize, m_halfZSize); glNormal3f(-1.0f, 0.0f, 0.0f); // NE
        glVertex3f(-m_halfXSize, m_halfYSize,-m_halfZSize); glNormal3f(-1.0f, 0.0f, 0.0f); // NW
        glVertex3f(-m_halfXSize,-m_halfYSize,-m_halfZSize); glNormal3f(-1.0f, 0.0f, 0.0f); // SW
        glVertex3f(-m_halfXSize,-m_halfYSize, m_halfZSize); glNormal3f(-1.0f, 0.0f, 0.0f); // SE
    }

    glEnd();

    if (texture2DOn) {
        glEnable(GL_TEXTURE_2D);
    }

    if (colorMaterialOn) {
        glEnable(GL_COLOR_MATERIAL);
    }
    else {
        glDisable(GL_COLOR_MATERIAL);
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
