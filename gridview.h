#ifndef GRID_VIEW_H
#define GRID_VIEW_H

#include <QtCore>
#include <QtGui>
#include <GL/glew.h>
#include <QGLWidget>

namespace Langmuir
{

  class GridViewGL:public QGLWidget
  {
  Q_OBJECT public:
    GridViewGL(QWidget * parent);
   ~GridViewGL();
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

   public slots:
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);
    void setXTranslation(float length);
    void setYTranslation(float length);
    void setZTranslation(float length);
    void timerUpdateGL();

   signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);
    void xTranslationChanged(float length);
    void yTranslationChanged(float length);
    void zTranslationChanged(float length);

   protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);

   private:
    void NormalizeAngle(int &angle);
    int xRot;
    int yRot;
    int zRot;
    float xTran;
    float yTran;
    float zTran;
    float xDelta;
    float yDelta;
    float zDelta;
    QPoint lastPos;
    QTimer *timer;
  };

  class MainWindow:public QWidget
  {
  Q_OBJECT public:
    MainWindow();

    private:
     GridViewGL * glWidget;
  };

}

#endif
