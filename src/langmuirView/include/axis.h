#ifndef AXIS_H
#define AXIS_H

#include "sceneobject.h"

/**
 * @brief A class to represent an xyz axis
 */
class Axis : public SceneObject
{
    Q_OBJECT
public:
    /**
     * @brief create the Axis
     * @param viewer the viewer
     * @param parent QObject this belongs to
     */
    explicit Axis(LangmuirViewer &viewer, QObject *parent = 0);

signals:
    /**
     * @brief signal that the color of x-axis has changed
     * @param color value of color
     */
    void xColorChanged(QColor color);

    /**
     * @brief signal that the color of y-axis has changed
     * @param color value of color
     */
    void yColorChanged(QColor color);

    /**
     * @brief signal that the color of z-axis has changed
     * @param color value of color
     */
    void zColorChanged(QColor color);

    /**
     * @brief signal that the axes radius has changed
     * @param value value of radius
     */
    void radiusChanged(double value);

    /**
     * @brief signal that the axes length has changed
     * @param value value of length
     */
    void lengthChanged(double value);

public slots:
    /**
     * @brief set the color of the x-axis
     * @param color color to set
     */
    void setXColor(QColor color);

    /**
     * @brief set the color of the x-axis
     * @param color color to set
     */
    void setYColor(QColor color);

    /**
     * @brief set the color of the x-axis
     * @param color color to set
     */
    void setZColor(QColor color);

    /**
     * @brief set the radius of axes
     * @param value radius to set
     */
    void setRadius(double value);

    /**
     * @brief set the length of axes
     * @param value length to set
     */
    void setLength(double value);

    /**
     * @brief make signal/slot connections
     */
    virtual void makeConnections();

protected:
    /**
     * @brief initialize object
     */
    virtual void init();

    /**
     * @brief perform OpenGL drawing operations
     */
    virtual void draw();

    //! axes radius
    double aradius;

    //! axes length
    double alength;

    //! color of x-axis
    QColor x_color;

    //! color of y-axis
    QColor y_color;

    //! color of z-axis
    QColor z_color;
};

#endif // AXIS_H
