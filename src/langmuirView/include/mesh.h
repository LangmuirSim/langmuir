#ifndef MESH_H
#define MESH_H

#include "sceneobject.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QVector>

/**
 * @brief A class to represent a mesh
 */
class Mesh : public SceneObject
{
    Q_OBJECT
public:
    /**
     * @brief create the Mesh
     * @param viewer the viewer
     * @param parent QObject this belongs to
     */
    explicit Mesh(LangmuirViewer &viewer, QObject *parent = 0);

    /**
      * @brief destroy the Mesh
      */
    ~Mesh();

    /**
     * @brief get color A
     */
    const QColor& getColorA() const;

    /**
     * @brief get color B
     */
    const QColor& getColorB() const;

signals:
    /**
     * @brief signal that color A of has changed
     * @param color value of color
     */
    void colorAChanged(QColor color);

    /**
     * @brief signal that color B of has changed
     * @param color value of color
     */
    void colorBChanged(QColor color);

    /**
     * @brief signal that the mesh has changed
     */
    void meshChanged();

public slots:
    /**
     * @brief make signal/slot connections
     */
    virtual void makeConnections();

    /**
     * @brief set color A
     * @param color color to set
     */
    void setColorA(QColor color);

    /**
     * @brief set color B
     * @param color color to set
     */
    void setColorB(QColor color);

    /**
     * @brief set up the mesh on the GPU
     * @param vertices vertex buffer
     * @param normals normal buffer
     * @param indices index buffer
     */
    void setMesh(QVector<GLfloat> &vertices, QVector<GLfloat>& normals, QVector<GLuint> &indices);

protected:
    /**
     * @brief initialize object
     */
    virtual void init();

    /**
     * @brief perform OpenGL drawing operations
     */
    virtual void draw();

    //! vertices buffer
    QOpenGLBuffer *m_verticesVBO;

    //! normals buffer
    QOpenGLBuffer *m_normalsVBO;

    //! index buffer CW
    QOpenGLBuffer *m_indexVBO;

    //! color of side A
    QColor m_colorA;

    //! color of side B
    QColor m_colorB;

    //! number of vertices (3 * number of points)
    unsigned int m_numVertices;

    //! index count
    unsigned int m_numIndices;
};

#endif // MESH_H
