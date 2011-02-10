#include "gridview.h"

namespace Langmuir
{

  GridViewGL::GridViewGL (QWidget * parent): QGLWidget (parent)
  {

  }

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
   glewInit();
/*
   glShadeModel (GL_SMOOTH);
   glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
   glClearDepth (1.0f);
   glEnable (GL_DEPTH_TEST);
   glDepthFunc (GL_LEQUAL);
   glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

   program = new QGLShaderProgram(this);
   program->addShaderFromSourceCode(QGLShader::Vertex,
     "uniform mat4 pMatrix;\n"
     "uniform mat4 mMatrix;\n"
     "uniform mat4 vMatrix;\n"
     "attribute vec4 vertex;\n"
     "void main(void)\n"
     "{\n"
     " gl_Position = pMatrix * vertex;\n"
     "}");
   program->addShaderFromSourceCode(QGLShader::Fragment,
     "void main(void)\n"
     "{\n"
     " gl_FragColor = vec4(1.0,0.0,0.0,0.0);\n"
     "}");
   program->link();

   viewMatrix.setToIdentity();
   modelMatrix.setToIdentity();
   projectionMatrix.setToIdentity();
*/
   QVector<float> pos(16,0);
   QVector<float> col(16,0);

    pos[0] = -1.0;  pos[1] =  1.0;  pos[2] =  0.0;  pos[3] = 1.0;
    col[0] =  1.0;  col[1] =  1.0;  col[2] =  0.0;  col[3] = 1.0;
    pos[4] =  1.0;  pos[5] =  1.0;  pos[6] =  0.0;  pos[7] = 1.0;
    col[4] =  0.0;  col[5] =  0.0;  col[6] =  1.0;  col[7] = 1.0;
    pos[8] =  1.0;  pos[9] = -1.0; pos[10] =  0.0; pos[11] = 1.0;
    col[8] =  0.0;  col[9] =  1.0; col[10] =  0.0; col[11] = 1.0;
   pos[12] = -1.0; pos[13] = -1.0; pos[14] =  0.0; pos[15] = 1.0;
   col[12] =  1.0; col[13] =  0.0; col[14] =  0.0; col[15] = 1.0;

   glGenBuffers(1,&vbuffer);
   glBindBuffer(GL_ARRAY_BUFFER,vbuffer);
   glBufferData(GL_ARRAY_BUFFER,pos.size()*sizeof(float),&pos[0],GL_DYNAMIC_DRAW);
   glBindBuffer(GL_ARRAY_BUFFER,0);

   glGenBuffers(1,&cbuffer);
   glBindBuffer(GL_ARRAY_BUFFER,cbuffer);
   glBufferData(GL_ARRAY_BUFFER,col.size()*sizeof(float),&col[0],GL_DYNAMIC_DRAW);
   glBindBuffer(GL_ARRAY_BUFFER,0);

  }

  void GridViewGL::resizeGL (int w, int h)
  {
   glViewport(0,0,w,h);
/*
   projectionMatrix.setToIdentity();
   projectionMatrix.perspective(60.0,qreal(w)/qreal(h),1.0,100.0); 
*/
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(45.0f,(GLfloat)w/(GLfloat)h,0.1f,100.0f);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
  }

  void GridViewGL::paintGL ()
  {
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glLoadIdentity();
   glTranslatef(0.0,0.0,-5.0);
   glEnable(GL_POINT_SMOOTH);
   glPointSize(5.0);

   glBindBuffer(GL_ARRAY_BUFFER,vbuffer);
   glBindBuffer(GL_ARRAY_BUFFER,cbuffer);

   glVertexPointer(4,GL_FLOAT,0,0);
   glColorPointer(4,GL_FLOAT,0,0);

   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_COLOR_ARRAY);

   glDrawArrays(GL_POINTS,0,4);

   glDisableClientState(GL_COLOR_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);

   glBindBuffer(GL_ARRAY_BUFFER,0);
/*
    viewMatrix.setToIdentity();
    modelMatrix.setToIdentity();

    viewMatrix.translate(0,0,-5.0);

    glEnable(GL_POINT_SMOOTH);
    glPointSize(5.0);

    program->bind();
    program->enableAttributeArray("vertex");
    program->setAttributeBuffer("vertex",GL_FLOAT,0,4);
    program->setUniformValue("pMatrix",projectionMatrix);
    program->setUniformValue("mMatrix",modelMatrix);
    program->setUniformValue("vMatrix",viewMatrix);

    glBindBuffer(GL_ARRAY_BUFFER,vbuffer);
    glBindBuffer(GL_ARRAY_BUFFER,cbuffer); 

    glVertexPointer(4,GL_FLOAT,0,0);
    glColorPointer(4,GL_FLOAT,0,0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glDrawArrays(GL_POINTS,0,4);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    program->disableAttributeArray("vertex");
    program->release();
*/
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
