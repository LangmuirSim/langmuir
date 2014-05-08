#ifndef GRID_H
#define GRID_H

#include "sceneobject.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QVector>

/**
 * @brief A class to represent simulation grid
 */
class Grid : public SceneObject
{
    Q_OBJECT
public:
    /**
     * @brief create the Grid
     * @param viewer the viewer
     * @param parent QObject this belongs to
     */
    explicit Grid(LangmuirViewer &viewer, QObject *parent = 0);

    /**
      * @brief destroy the Grid
      */
    ~Grid();

signals:
    /**
     * @brief signal that the color of has changed
     * @param color value of color
     */
    void colorChanged(QColor color);

    /**
     * @brief signal that the grid has changed
     */
    void gridChanged();

public slots:
    /**
     * @brief make signal/slot connections
     */
    virtual void makeConnections();

    /**
     * @brief create the grid using dimensions
     * @param xsize size of grid in x-direction
     * @param ysize size of grid in y-direction
     * @param zsize size of grid in z-direction
     */
    void setDimensions(int xsize, int ysize, int zsize);

    /**
     * @brief set the color
     * @param color color to set
     */
    void setColor(QColor color);

protected:
    /**
     * @brief initialize object
     */
    virtual void init();

    /**
     * @brief perform OpenGL drawing operations
     */
    virtual void draw();

    /**
     * @brief load the shaders
     */
    void initShaders();

    //! OpenGL shading pipeline
    QOpenGLShaderProgram m_shader1; // lines pipeline

    //! vertices buffer
    QOpenGLBuffer *m_verticesVBO;

    //! total number of points (# vertices / 3)
    unsigned int m_numPoints;

    //! color of grid
    QColor m_color;
};

#endif // GRID_H
