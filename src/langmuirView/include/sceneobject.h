#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <QObject>

class SceneObject : public QObject
{
    Q_OBJECT
public:
    explicit SceneObject(QObject *parent = 0);
    bool isVisible();
    void render();

signals:
    void visibleChanged(bool drawn);

public slots:
    void toggleVisible();
    void setVisible(bool draw = true);

protected:
    virtual void draw();
    virtual void preDraw();
    virtual void postDraw();
    bool visible_;
};

#endif // SCENEOBJECT_H
