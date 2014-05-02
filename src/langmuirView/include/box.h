#ifndef BOX_H
#define BOX_H

#include "sceneobject.h"
#include <QOpenGLBuffer>

class Box : public SceneObject
{
    Q_OBJECT
public:
    explicit Box(LangmuirViewer &viewer, QObject *parent = 0);
    ~Box();

    enum Face {
        None  =  1,
        North =  2,
        South =  4,
        East  =  8,
        West  = 16,
        Front = 32,
        Back  = 64,
        All   = North | South | East | West | Front | Back
    };
    Q_DECLARE_FLAGS(Faces, Face)
    Q_FLAGS(Faces)

signals:
    void colorChanged(QColor color);
    void xSizeChanged(double value);
    void ySizeChanged(double value);
    void zSizeChanged(double value);
    void imageOnChanged(bool drawn);
    void facesChanged(Faces faces);

public slots:
    virtual void makeConnections();
    void setColor(QColor color);
    void setXSize(double value);
    void setYSize(double value);
    void setZSize(double value);

    void setFaces(Faces faces);
    void loadImage(const QImage &image);
    void showImage(bool on=true);
    void toggleImage();

protected:
    virtual void init();
    virtual void draw();    
    QColor m_color;
    double m_xsize;
    double m_ysize;
    double m_zsize;
    GLuint m_imageID;
    bool   m_imageOn;
    Faces  m_faces;

private:
    void applyColor(const QColor& color);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Box::Faces)

#endif // BOX_H
