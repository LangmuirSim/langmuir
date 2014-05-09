#include "langmuirviewer.h"
#include "light.h"
#include "color.h"

Light::Light(GLenum lightID, LangmuirViewer &viewer, QObject *parent) :
    SceneObject(viewer, parent), m_lightID(lightID), m_enabled(false)
{
    init();
}

Light::~Light()
{
}

const QColor& Light::getAColor() const
{
    return m_acolor;
}

const QColor& Light::getDColor() const
{
    return m_dcolor;
}

const QColor& Light::getSColor() const
{
    return m_scolor;
}

GLenum Light::getLightID() const
{
    return m_lightID;
}

bool Light::isEnabled() const
{
    return m_enabled;
}

void Light::init() {

    m_position = QVector4D(-1, -1, -1, -1);
    m_acolor   = Qt::blue;
    m_dcolor   = Qt::blue;
    m_scolor   = Qt::blue;

    setPosition(0.0, 0.0, 0.0, 0.0);
    setAColor(Qt::black);
    setDColor(Qt::black);
    setSColor(Qt::black);
    setVisible(false);

    emit lightIDChanged(m_lightID);
    emit enabledChanged(m_enabled);
}

void Light::draw()
{
    m_viewer.drawLightSource(m_lightID, 1.0f);
}

void Light::setPosition(float x, float y, float z, float w)
{
    setPosition(QVector4D(x, y, z, w));
}

void Light::setPosition(QVector4D value)
{
    if (value != m_position) {
        m_position = value;

        updatePosition();

        emit positionChanged(m_position);
    }
}

void Light::setAColor(QColor color)
{
    if (color != m_acolor) {
        m_acolor = color;

        updateAColor();

        emit aColorChanged(m_acolor);
    }
}

void Light::setDColor(QColor color)
{
    if (color != m_dcolor) {
        m_dcolor = color;

        updateDColor();

        emit dColorChanged(m_dcolor);
    }
}

void Light::setSColor(QColor color)
{
    if (color != m_scolor) {
        m_scolor = color;

        updateSColor();

        emit sColorChanged(m_scolor);
    }
}

void Light::setLightID(GLuint lightID)
{
    if (lightID != m_lightID) {

        bool wasEnabled = m_enabled;

        if (wasEnabled)
        {
            glDisable(m_lightID);
        }

        m_lightID = lightID;

        if (wasEnabled)
        {
            glEnable(m_lightID);
        }

        emit lightIDChanged(m_lightID);
    }
}

void Light::setEnabled(bool enabled)
{
    if (enabled != m_enabled) {
        m_enabled = enabled;

        if (m_enabled)
        {
            glEnable(m_lightID);
        }
        else
        {
            glDisable(m_lightID);
        }

        emit enabledChanged(m_enabled);
    }
}

void Light::toggle()
{
    setEnabled(!m_enabled);
}

void Light::updatePosition()
{
    glLightfv(m_lightID, GL_POSITION, &m_position[0]);
}

void Light::updateAColor()
{
    static float color[4];
    color::qColorToArray4(m_acolor, color);
    glLightfv(m_lightID, GL_AMBIENT, color);
}

void Light::updateSColor()
{
    static float color[4];
    color::qColorToArray4(m_scolor, color);
    glLightfv(m_lightID, GL_SPECULAR, color);
}

void Light::updateDColor()
{
    static float color[4];
    color::qColorToArray4(m_dcolor, color);
    glLightfv(m_lightID, GL_DIFFUSE, color);
}

void Light::makeConnections()
{
    SceneObject::makeConnections();
    connect(this, SIGNAL(positionChanged(QVector4D)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(aColorChanged(QColor)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(dColorChanged(QColor)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(sColorChanged(QColor)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(lightIDChanged(GLuint)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(enabledChanged(bool)), &m_viewer, SLOT(updateGL()));
}
