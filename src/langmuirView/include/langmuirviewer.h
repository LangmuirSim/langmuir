#ifndef LANGMUIRVIEWER_H
#define LANGMUIRVIEWER_H

#include <QGLViewer/qglviewer.h>

class LangmuirViewer : public QGLViewer
{
    Q_OBJECT
public:
    explicit LangmuirViewer(QWidget *parent = 0);

protected:
    virtual void draw();
    virtual void init();
    virtual void animate();
    virtual QString helpString() const;
};

#endif // LANGMUIRVIEWER_H
