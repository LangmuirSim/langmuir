#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <QVector>
#include <QObject>
#include <QColor>

#ifdef Q_OS_MAC
    #include <OpenGL/glu.h>
#else
    #include <GL/glu.h>
#endif

class LangmuirViewer;

class SceneObject : public QObject
{
    Q_OBJECT
public:
    explicit SceneObject(LangmuirViewer &viewer, QObject *parent = 0);
    bool isVisible();
    void render();

signals:
    void visibleChanged(bool drawn);

public slots:
    void toggleVisible();
    void setVisible(bool draw = true);
    virtual void makeConnections();

protected:
    virtual void init();
    virtual void draw();
    virtual void preDraw();
    virtual void postDraw();
    LangmuirViewer &m_viewer;
    static float m_float4[4];
    bool visible_;
};

#endif // SCENEOBJECT_H
