#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <QVector>
#include <QObject>
#include <QColor>

class LangmuirViewer;

class SceneObject : public QObject
{
    Q_OBJECT
public:
    explicit SceneObject(LangmuirViewer &viewer, QObject *parent = 0);
    void qColorToFloat3(QColor qcolor, float *color);
    void qColorToFloat4(QColor qcolor, float *color);
    bool isVisible();
    void render();

signals:
    void visibleChanged(bool drawn);

public slots:
    void toggleVisible();
    void setVisible(bool draw = true);

protected:
    virtual void init();
    virtual void draw();
    virtual void preDraw();
    virtual void postDraw();
    LangmuirViewer &m_viewer;
    bool visible_;
};

#endif // SCENEOBJECT_H
