#include "gridview.h"

namespace Langmuir
{

  QSize GridViewGL::minimumSizeHint () const
  {
    return QSize (200, 200);
  }

  QSize GridViewGL::sizeHint () const
  {
    return QSize (500, 500);
  }

  void GridViewGL::initializeGL ()
  {
    glShadeModel (GL_SMOOTH);
    glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth (1.0f);
    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  }

  void GridViewGL::resizeGL (int w, int h)
  {
    glViewport (0, 0, w, h);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glOrtho (-0.5, +0.5, -0.5, +0.5, 4.0, 15.0);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
  }

  void GridViewGL::paintGL ()
  {
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity ();
  }

  MainWindow::MainWindow ()
  {
    glWidget = new GridViewGL (this);
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget (glWidget);
    setLayout (mainLayout);
    setWindowTitle (tr ("Langmuir View"));
  }

}
