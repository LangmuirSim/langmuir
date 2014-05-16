#ifndef LIGHT_H
#define LIGHT_H

#include "sceneobject.h"

#include <QVector4D>

/**
 * @brief A class to represent a light source
 */
class Light : public SceneObject
{
    Q_OBJECT
public:
    explicit Light(GLenum lightID, LangmuirViewer &viewer, QObject *parent = 0);
    ~Light();

    /**
     * @brief get ambient color
     */
    const QColor& getAColor() const;

    /**
     * @brief get diffuse color
     */
    const QColor& getDColor() const;

    /**
     * @brief get specular color
     */
    const QColor& getSColor() const;

    /**
     * @brief get the OpenGL light id
     */
    GLenum getLightID() const;

    /**
     * @brief find out if the light is enabled
     */
    bool isEnabled() const;

    /**
     * @brief get position of light
     */
    const QVector4D& getPosition() const;

signals:
    /**
     * @brief signal that the position changed
     * @param position value of position
     */
    void positionChanged(QVector4D position);

    /**
     * @brief signal that the ambient color of has changed
     * @param color value of color
     */
    void aColorChanged(QColor color);

    /**
     * @brief signal that the diffuse color of has changed
     * @param color value of color
     */
    void dColorChanged(QColor color);

    /**
     * @brief signal that the specular color of has changed
     * @param color value of color
     */
    void sColorChanged(QColor color);

    /**
     * @brief signal that the light has been enabled/disabled
     * @param enabled true if light has been enabled
     */
    void enabledChanged(bool enabled);

    /**
     * @brief signal that the OpenGL light ID has changed
     * @param lightID value of OpenGL light ID
     */
    void lightIDChanged(GLuint lightID);

public slots:
    /**
     * @brief make signal/slot connections
     */
    virtual void makeConnections();

    /**
     * @brief set position of light
     * @param x x-position
     * @param y y-position
     * @param z z-position
     * @param w w-position
     */
    void setPosition(float x, float y, float z, float w=0.0);

    /**
     * @brief set position of light
     * @param value position to set
     */
    void setPosition(QVector4D value);

    /**
     * @brief set the ambient color
     * @param color color to set
     */
    void setAColor(QColor color);

    /**
     * @brief set the diffuse color
     * @param color color to set
     */
    void setDColor(QColor color);

    /**
     * @brief set the specular color
     * @param color color to set
     */
    void setSColor(QColor color);

    /**
     * @brief set the OpenGL light ID
     * @param lightID OpenGL light ID to use
     */
    void setLightID(GLuint lightID);

    /**
     * @brief enabled or disable the light
     * @param enabled true if the light is to be enabled
     */
    void setEnabled(bool enabled);

    /**
     * @brief toggle the light between enabled/disabled
     */
    void toggle();

    //! use OpenGL commands
    void updatePosition();

    //! use OpenGL commands
    void updateAColor();

    //! use OpenGL commands
    void updateSColor();

    //! use OpenGL commands
    void updateDColor();

protected:
    /**
     * @brief initialize object
     */
    virtual void init();

    /**
     * @brief perform OpenGL drawing operations
     */
    virtual void draw();

    //! location of light
    QVector4D m_position;

    //! ambient color
    QColor m_acolor;

    //! diffuse color
    QColor m_dcolor;

    //! specular color
    QColor m_scolor;

    //! OpenGL light ID
    GLenum m_lightID;

    //! true if light is on
    bool m_enabled;
};

#endif // LIGHT_H
