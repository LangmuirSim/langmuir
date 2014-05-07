#ifndef POINTCLOUD_H
#define POINTCLOUD_H

#include "sceneobject.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QVector>

class PointCloud : public SceneObject
{
    Q_OBJECT
public:
    explicit PointCloud(LangmuirViewer &viewer, QObject *parent = 0);
    ~PointCloud();

    enum Mode {
        Points =  1,
        Cubes  =  2,
    };
    Q_DECLARE_FLAGS(Modes, Mode)
    Q_FLAGS(Modes)

public:
    QVector<float>& vertices();
    unsigned int getMaxPoints();
    unsigned int getMaxRender();

signals:
    void maxRenderChanged(unsigned int value);
    void pointSizeChanged(float value);
    void colorChanged(QColor color);
    void modeChanged(Mode mode);
    void vboChanged();

public slots:
    virtual void makeConnections();
    void setMaxRender(unsigned int value);
    void setPointSize(float value);
    void setColor(QColor color);
    void setMode(Mode mode);
    void updateVBO();

protected:
    virtual void init();
    virtual void draw();
    void initShaders();

    QOpenGLShaderProgram m_shader1; // point pipeline
    QOpenGLShaderProgram m_shader2; // cubes pipeline

    QOpenGLBuffer *m_verticesVBO;
    QVector<float> m_vertices;
    unsigned int m_maxPoints;
    unsigned int m_maxRender;
    float m_pointSize;
    QColor m_color;
    Mode m_mode;
};

#endif // POINTCLOUD_H
