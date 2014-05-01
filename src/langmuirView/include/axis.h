#ifndef AXIS_H
#define AXIS_H

#include "sceneobject.h"

class Axis : public SceneObject
{
    Q_OBJECT
public:
    explicit Axis(LangmuirViewer &viewer, QObject *parent = 0);

signals:
    void xColorChanged(QColor color);
    void yColorChanged(QColor color);
    void zColorChanged(QColor color);
    void radiusChanged(double value);
    void lengthChanged(double value);

public slots:
    void setXColor(QColor color);
    void setYColor(QColor color);
    void setZColor(QColor color);
    void setRadius(double value);
    void setLength(double value);
    virtual void makeConnections();

protected:
    virtual void init();
    virtual void draw();
    double aradius;
    double alength;
    QColor x_color;
    QColor y_color;
    QColor z_color;
    float color[4];
};

#endif // AXIS_H
