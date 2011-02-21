#include "gridview.h"

namespace Langmuir
{

  GridViewGL::GridViewGL(QWidget * parent):QGLWidget(parent)
  {
   //setMouseTracking(true);
   setFocusPolicy(Qt::StrongFocus);
  }

  GridViewGL::~GridViewGL()
  {
  }

  QSize GridViewGL::minimumSizeHint() const
  {
    return QSize(200, 200);
  }

  QSize GridViewGL::sizeHint() const
  {
    return QSize(500, 500);
  }

  void GridViewGL::initializeGL()
  {
    glewInit();
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    xRot  = 0;
    yRot  = 0;
    zRot  = 0;
    xTran = 0;
    yTran = 0;
    zTran = -10;
    xDelta = 1.0;
    yDelta = 1.0;
    zDelta = 1.0;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerUpdateGL()));
    timer->start(1000);
  }

  void GridViewGL::timerUpdateGL()
  {
   updateGL();
  }

  void GridViewGL::NormalizeAngle(int &angle)
  {
    while(angle < 0)
      angle += 360 * 16;
    while(angle > 360 * 16)
      angle -= 360 * 16;
  }

  void GridViewGL::setXRotation(int angle)
  {
    NormalizeAngle(angle);
    if(angle != xRot)
      {
        xRot = angle;
        emit xRotationChanged(angle);
        updateGL();
      }
  }

  void GridViewGL::setYRotation(int angle)
  {
    NormalizeAngle(angle);
    if(angle != yRot)
      {
        yRot = angle;
        emit yRotationChanged(angle);
        updateGL();
      }
  }

  void GridViewGL::setZRotation(int angle)
  {
    NormalizeAngle(angle);
    if(angle != zRot)
      {
        zRot = angle;
        emit zRotationChanged(angle);
        updateGL();
      }
  }

  void GridViewGL::setXTranslation(float length)
  {
    if ( length != xTran )
     {
      xTran = length;
      emit xTranslationChanged(length);
      updateGL();
     }
  }

  void GridViewGL::setYTranslation(float length)
  {
    if ( length != yTran )
     {
      yTran = length;
      emit yTranslationChanged(length);
      updateGL();
     }
  }

  void GridViewGL::setZTranslation(float length)
  {
    if ( length != zTran )
     {
      zTran = length;
      emit zTranslationChanged(length);
      updateGL();
     }
  }

  void GridViewGL::mousePressEvent(QMouseEvent * event)
  {
    lastPos = event->pos();
  }

  void GridViewGL::mouseMoveEvent(QMouseEvent * event)
  {
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if(event->buttons() & Qt::RightButton)
      {
        setXRotation(xRot + 8 * dy);
        setYRotation(yRot + 8 * dx);
      }
    else if(event->buttons() & Qt::LeftButton)
      {
        setXTranslation(xTran+0.01*dx);
        setYTranslation(yTran-0.01*dy);
        //setXRotation(xRot + 8 * dy);
        //setZRotation(zRot + 8 * dx);
      }
    lastPos = event->pos();
  }

  void GridViewGL::wheelEvent(QWheelEvent *event)
  {
   setZTranslation(zTran + zDelta * event->delta()/120.0);
  }

  void GridViewGL::keyPressEvent(QKeyEvent *event)
  {
   switch ( event->key() )
   {
    case Qt::Key_Left:
    {
     setXTranslation(xTran-xDelta);
     break;
    }
    case Qt::Key_Right:
    {
     setXTranslation(xTran+xDelta);
     break;
    }
    case Qt::Key_Up:
    {
     setYTranslation(yTran+yDelta);
     break;
    }
    case Qt::Key_Down:
    {
     setYTranslation(yTran-yDelta);
     break;
    }
    case Qt::Key_Plus:
    {
     setZTranslation(zTran+zDelta);
     break;
    }
    case Qt::Key_Minus:
    {
     setZTranslation(zTran-zDelta);
     break;
    }
    case Qt::Key_Escape:
    {
     QWidget *parent = parentWidget();
     parent->close();
     break;
    }
    default:
    {
     break;
    }
   }
  }

  void GridViewGL::resizeGL(int w, int h)
  {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f,(GLfloat)w/(GLfloat)h,0.1f,100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  }

  void GridViewGL::paintGL()
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(xTran,yTran,zTran);
    glRotatef(xRot/16.0,1.0,0.0,0.0);
    glRotatef(yRot/16.0,0.0,1.0,0.0);
    glRotatef(zRot/16.0,0.0,0.0,1.0);
    //glEnable(GL_POINT_SMOOTH);
    //glPointSize(5.0);

    glBegin(GL_QUADS);
     glColor3f(0.0f,1.0f,0.0f);
     glVertex3f( 1.0f, 1.0f,-1.0f);
     glVertex3f(-1.0f, 1.0f,-1.0f);
     glVertex3f(-1.0f, 1.0f, 1.0f);
     glVertex3f( 1.0f, 1.0f, 1.0f);
     glColor3f(1.0f,0.5f,0.0f);
     glVertex3f( 1.0f,-1.0f, 1.0f);
     glVertex3f(-1.0f,-1.0f, 1.0f);
     glVertex3f(-1.0f,-1.0f,-1.0f);
     glVertex3f( 1.0f,-1.0f,-1.0f);
     glColor3f(1.0f,0.0f,0.0f);
     glVertex3f( 1.0f, 1.0f, 1.0f);
     glVertex3f(-1.0f, 1.0f, 1.0f);
     glVertex3f(-1.0f,-1.0f, 1.0f);
     glVertex3f( 1.0f,-1.0f, 1.0f);
     glColor3f(1.0f,1.0f,0.0f);
     glVertex3f( 1.0f,-1.0f,-1.0f);
     glVertex3f(-1.0f,-1.0f,-1.0f);
     glVertex3f(-1.0f, 1.0f,-1.0f);
     glVertex3f( 1.0f, 1.0f,-1.0f);
     glColor3f(0.0f,0.0f,1.0f);
     glVertex3f(-1.0f, 1.0f, 1.0f);
     glVertex3f(-1.0f, 1.0f,-1.0f);
     glVertex3f(-1.0f,-1.0f,-1.0f);
     glVertex3f(-1.0f,-1.0f, 1.0f);
     glColor3f(1.0f,0.0f,1.0f);
     glVertex3f( 1.0f, 1.0f,-1.0f);
     glVertex3f( 1.0f, 1.0f, 1.0f);
     glVertex3f( 1.0f,-1.0f, 1.0f);
     glVertex3f( 1.0f,-1.0f,-1.0f);
    glEnd();

  }

  MainWindow::MainWindow()
  {
    glWidget = new GridViewGL(this);
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(glWidget);
    setLayout(mainLayout);
    setWindowTitle(tr("Langmuir View"));
  }

}
