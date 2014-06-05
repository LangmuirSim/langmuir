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
     * @brief The rendering mode for the cloud
     */
    enum Mode {
        Single      =  1, //! render mesh using single color
        SingleAlpha =  2, //! render mesh using single color with alpha blending
        Double      =  3, //! render mesh using two colors
        DoubleAlpha =  4, //! render mesh using two colors with alpha blending
        Shader1     =  5, //! render mesh with shader1
        Shader2     =  6, //! render mesh with shader2
    };
    Q_DECLARE_FLAGS(Modes, Mode)
    Q_FLAGS(Modes)

    /**
     * @brief get color A
     */
    const QColor& getColorA() const;

    /**
     * @brief get color B
     */
    const QColor& getColorB() const;

    /**
     * @brief get render mode
     */
    Mode getMode() const;

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

    /**
     * @brief signal that the render mode has changed
     * @param mode value of rendering mode
     */
    void modeChanged(Mesh::Mode mode);

    /**
     * @brief signal that the render mode has changed
     * @param mode value of rendering mode
     */
    void modeChanged(QString mode);

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
    void setMesh(const QVector<float> &vertices, const QVector<float>& normals, const QVector<unsigned int> &indices);

    /**
     * @brief set the mode
     * @param mode mode to set
     */
    void setMode(Mesh::Mode mode);

    /**
     * @brief clear GPU buffers
     */
    void clear();

    /**
     * @brief convert Mode to string
     * @param mode mode enum
     */
    static QString modeToQString(Mode mode);

    /**
     * @brief convert string to Mode enum
     * @param string mode string
     */
    static Mode QStringToMode(QString string);

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

    //! render function
    void drawSingle();

    //! render function
    void drawSingleAlpha();

    //! render function
    void drawDouble();

    //! render function
    void drawDoubleAlpha();

    //! render function
    void drawShader1();

    //! render function
    void drawShader2();

    //! tesselation shader
    QOpenGLShaderProgram m_shader1;

    //! tesselation shader
    QOpenGLShaderProgram m_shader2;

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

    //! rendering mode
    Mode m_mode;

    //! shader2 ok to use
    bool m_shader2OK;
};

Q_DECLARE_METATYPE(Mesh::Mode);

#endif // MESH_H
