#ifndef BOX_H
#define BOX_H

#include "sceneobject.h"

class Box : public SceneObject
{
    Q_OBJECT
public:
    explicit Box(LangmuirViewer &viewer, QObject *parent = 0);

signals:
    void colorChanged(QColor color);

public slots:
    void setColor(QColor color);

protected:
    virtual void init();
    virtual void draw();
    QColor m_color;
};

#endif // BOX_H
