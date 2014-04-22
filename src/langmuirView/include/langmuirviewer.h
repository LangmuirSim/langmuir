#ifndef LANGMUIRVIEWER_H
#define LANGMUIRVIEWER_H

#include <QGLViewer/qglviewer.h>

class LangmuirViewer : public QGLViewer
{
    Q_OBJECT
public:
    explicit LangmuirViewer(QWidget *parent = 0);
    bool cornerAxisIsDrawn();

signals:
    void cornerAxisIsDrawnChanged(bool drawn);

public slots:
    void toggleCornerAxisIsDrawn();
    void setCornerAxisIsDrawn(bool draw = true);

protected:
    virtual void draw();
    virtual void postDraw();
    virtual void init();
    virtual void animate();
    virtual QString helpString() const;

    bool cornerAxisIsDrawn_;
    virtual void drawCornerAxis();
};

#endif // LANGMUIRVIEWER_H
