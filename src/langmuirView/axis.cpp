#include "langmuirviewer.h"
#include "color.h"
#include "axis.h"

#include <QDebug>

namespace LangmuirView {

Axis::Axis(LangmuirViewer &viewer, QObject *parent) :
    SceneObject(viewer, parent)
{
    init();
}

const QColor& Axis::getXColor() const
{
    return m_xcolor;
}

const QColor& Axis::getYColor() const
{
    return m_ycolor;
}

const QColor& Axis::getZColor() const
{
    return m_zcolor;
}

double Axis::getLength() const
{
    return m_length;
}

double Axis::getRadius() const
{
    return m_radius;
}

void Axis::init() {
    m_xcolor = QColor::fromRgbF(1.0, 0.0, 0.0, 1.0);
    m_ycolor = QColor::fromRgbF(0.0, 1.0, 0.0, 1.0);
    m_zcolor = QColor::fromRgbF(0.0, 0.0, 1.0, 1.0);
    m_length = 1.00;
    m_radius = 0.01;

    emit xColorChanged(m_xcolor);
    emit yColorChanged(m_ycolor);
    emit zColorChanged(m_zcolor);
    emit lengthChanged(m_length);
    emit radiusChanged(m_radius);
}

void Axis::draw() {
    // lighting and color saved
    GLboolean lighting, colorMaterial, texture2DIsOn;
    glGetBooleanv(GL_LIGHTING, &lighting);
    glGetBooleanv(GL_TEXTURE_2D, &texture2DIsOn);
    glGetBooleanv(GL_COLOR_MATERIAL, &colorMaterial);

    // enable lighting
    glEnable(GL_LIGHTING);

    // color determined by glMaterial
    glDisable(GL_COLOR_MATERIAL);

    // turn off textures
    glDisable(GL_TEXTURE_2D);

    static float color[4];

    // x-axis
    color::qColorToArray4(m_xcolor, color);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glPushMatrix();
    glRotatef(0.0, 0.0, 1.0, 0.0);
    m_viewer.drawArrow(m_length, m_radius);
    glPopMatrix();

    // y-axis
    color::qColorToArray4(m_ycolor, color);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glPushMatrix();
    glRotatef(90.0, 0.0, 1.0, 0.0);
    QGLViewer::drawArrow(m_length, m_radius);
    glPopMatrix();

    // z-axix
    color::qColorToArray4(m_zcolor, color);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glPushMatrix();
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    QGLViewer::drawArrow(m_length, m_radius);
    glPopMatrix();

    // restore
    if (colorMaterial) {
        glEnable(GL_COLOR_MATERIAL);
    }
    else {
        glDisable(GL_COLOR_MATERIAL);
    }

    if (lighting) {
        glEnable(GL_LIGHTING);
    }
    else {
        glDisable(GL_LIGHTING);
    }

    if (texture2DIsOn) {
        glEnable(GL_TEXTURE_2D);
    }
}

void Axis::setXColor(QColor color)
{
    if (color != m_xcolor) {
        m_xcolor = color;
        emit xColorChanged(m_xcolor);
    }
}

void Axis::setYColor(QColor color)
{
    if (color != m_ycolor) {
        m_ycolor = color;
        emit yColorChanged(m_ycolor);
    }
}

void Axis::setZColor(QColor color)
{
    if (color != m_zcolor) {
        m_zcolor = color;
        emit zColorChanged(m_zcolor);
    }
}

void Axis::setRadius(double value)
{
    if (value != m_radius) {
        m_radius = value;
        emit radiusChanged(value);
    }
}

void Axis::setLength(double value)
{
    if (value != m_length) {
        m_length = value;
        emit lengthChanged(value);
    }
}

void Axis::makeConnections()
{
    SceneObject::makeConnections();
    connect(this, SIGNAL(xColorChanged(QColor)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(yColorChanged(QColor)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(zColorChanged(QColor)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(radiusChanged(double)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(lengthChanged(double)), &m_viewer, SLOT(updateGL()));
}

}
