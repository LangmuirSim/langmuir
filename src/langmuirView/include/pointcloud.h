#ifndef POINTCLOUD_H
#define POINTCLOUD_H

#include "sceneobject.h"

class PointCloud : public SceneObject
{
    Q_OBJECT
public:
    explicit PointCloud(LangmuirViewer &viewer, QObject *parent = 0);

signals:
    void colorChanged(QColor color);

public slots:
    virtual void makeConnections();
    void setColor(QColor color);

protected:
    virtual void init();
    virtual void draw();
    QColor m_color;
};

#endif // POINTCLOUD_H
