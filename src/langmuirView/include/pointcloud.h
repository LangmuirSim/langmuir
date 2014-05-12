#ifndef POINTCLOUD_H
#define POINTCLOUD_H

#include "sceneobject.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QVector>

/**
 * @brief A class to represent a point cloud
 */
class PointCloud : public SceneObject
{
    Q_OBJECT
public:
    /**
     * @brief create the PointCloud
     * @param viewer the viewer
     * @param parent QObject this belongs to
     */
    explicit PointCloud(LangmuirViewer &viewer, QObject *parent = 0);

    /**
      * @brief destroy the PointCloud
      */
    ~PointCloud();

    /**
     * @brief The rendering mode for the cloud
     */
    enum Mode {
        Points =  1, //! render points as OpenGL points
        Cubes  =  2, //! render points as cubes
    };
    Q_DECLARE_FLAGS(Modes, Mode)
    Q_FLAGS(Modes)

    /**
     * @brief get color
     */
    const QColor& getColor() const;

    /**
     * @brief get x length
     */
    float getPointSize() const;

    /**
     * @brief get render mode
     */
    Mode getMode() const;

public:
    /**
     * @brief get the list of vertices (CPU memory)
     * @warning please do not resize this vector
     */
    QVector<float>& vertices();

    /**
     * @brief get the maximum number of points that can be rendered
     */
    unsigned int getMaxPoints();

    /**
     * @brief get the number of points currently being rendered
     * @warning max points >= max rendered
     */
    unsigned int getMaxRender();

signals:
    /**
     * @brief signal that the maximum number of points has changed
     * @param value value of max
     */
    void maxPointsChanged(unsigned int value);

    /**
     * @brief signal that the maximum number of points rendered has changed
     * @param value value of max
     */
    void maxRenderChanged(unsigned int value);

    /**
     * @brief signal that the point size has changed
     * @param value value of point size
     */
    void pointSizeChanged(float value);

    /**
     * @brief signal that the color of has changed
     * @param color value of color
     */
    void colorChanged(QColor color);

    /**
     * @brief signal that the render mode has changed
     * @param mode value of rendering mode
     */
    void modeChanged(Mode mode);

public slots:
    /**
     * @brief make signal/slot connections
     */
    virtual void makeConnections();

    /**
     * @brief change the maximum number of points allowed
     * @param value max points to set
     *
     * This deletes the GPU memory and creates a new buffer.
     * Do not do it often.
     */
    void setMaxPoints(unsigned int value);

    /**
     * @brief change the maximum number of points to render
     * @param value max points to render
     *
     * Make sure this is less than the max points allowed.
     */
    void setMaxRender(unsigned int value);

    /**
     * @brief set the point size
     * @param value point size to set
     */
    void setPointSize(float value);

    /**
     * @brief set the color
     * @param color color to set
     */
    void setColor(QColor color);

    /**
     * @brief set the mode
     * @param mode mode to set
     */
    void setMode(Mode mode);

    /**
     * @brief update the GPU memory using CPU vertices
     *
     * First alter the CPU vertices, set the max render, then update the VBO.
     */
    void updateVBO();

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

    //! OpenGL shading pipeline for points
    QOpenGLShaderProgram m_shader1;

    //! OpenGL shading pipeline for cubes
    QOpenGLShaderProgram m_shader2;

    //! vertices buffer (GPU)
    QOpenGLBuffer *m_verticesVBO;

    //! vertices buffer (CPU)
    QVector<float> m_vertices;

    //! maximum number of points allowed (# vertices / 3)
    unsigned int m_maxPoints;

    //! maximum number of points rendered (less than or equal to max points)
    unsigned int m_maxRender;

    //! the size of points
    float m_pointSize;

    //! color of points
    QColor m_color;

    //! rendering mode
    Mode m_mode;
};

Q_DECLARE_METATYPE(PointCloud::Mode);

#endif // POINTCLOUD_H
