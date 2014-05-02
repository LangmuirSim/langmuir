#ifndef BOX_H
#define BOX_H

#include "sceneobject.h"
#include <QOpenGLBuffer>

class Box : public SceneObject
{
    Q_OBJECT
public:
    explicit Box(LangmuirViewer &viewer, QObject *parent = 0);

signals:
    void colorChanged(QColor color);
    void xSizeChanged(double value);
    void ySizeChanged(double value);
    void zSizeChanged(double value);

public slots:
    void setColor(QColor color);
    void setXSize(double value);
    void setYSize(double value);
    void setZSize(double value);
    virtual void makeConnections();

protected:
    virtual void init();
    virtual void draw();
    QColor m_color;
    double m_xsize;
    double m_ysize;
    double m_zsize;
};

#endif // BOX_H
