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

public:
    QVector<float>& vertices();
    unsigned int getMaxPoints();
    unsigned int getMaxRender();

signals:
    void maxRenderChanged(unsigned int value);
    void colorChanged(QColor color);
    void vboChanged();

public slots:
    virtual void makeConnections();
    void setMaxRender(unsigned int value);
    void setColor(QColor color);
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
    QColor m_color;
};

#endif // POINTCLOUD_H
