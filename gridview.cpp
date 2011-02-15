#include "gridview.h"

namespace Langmuir
{

  GridViewGL::GridViewGL (QWidget * parent): QGLWidget (parent)
  {

  }

  GridViewGL::~GridViewGL ()
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

   glGenVertexArrays(1,&varray);
   glBindVertexArray(varray);

   glGenBuffers(1,&vbuffer);
   glBindBuffer(GL_ARRAY_BUFFER,vbuffer);
   glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(float), &pos[0], GL_STATIC_DRAW);

   glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
   glEnableVertexAttribArray(0);

   glGenBuffers(1,&fbuffer);
   glBindBuffer(GL_ARRAY_BUFFER,fbuffer);
   glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(float), &col[0], GL_STATIC_DRAW);

   glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,0);
   glEnableVertexAttribArray(1);

   QFile vfile("shader.vert");
   if (!vfile.open(QIODevice::ReadOnly|QIODevice::Text)) qFatal("can not open shader file");
   QByteArray varray = vfile.readAll();
   vfile.close();
   char * vsource = new char[varray.size()];
   for ( int i = 0; i < varray.size(); i++ ) vsource[i] = varray[i];

   QFile ffile("shader.frag");
   if (!ffile.open(QIODevice::ReadOnly|QIODevice::Text)) qFatal("can not open shader file");
   QByteArray farray = ffile.readAll();
   ffile.close();
   char * fsource = new char[farray.size()];
   for ( int i = 0; i < farray.size(); i++ ) fsource[i] = farray[i];

   vshader = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(vshader,1,(const GLchar**)&vsource,0);
   glCompileShader(vshader);
   {
    int compiled = GL_TRUE;
    glGetShaderiv(vshader,GL_COMPILE_STATUS,&compiled);
    if ( compiled == GL_FALSE )
    {
     int maxLength = 0;
     glGetShaderiv(vshader,GL_INFO_LOG_LENGTH,&maxLength);
     char * Log = new char [ maxLength ];
     glGetShaderInfoLog(vshader,maxLength,&maxLength,Log);
     qDebug() << Log;
     delete Log;
     qFatal("can not compile vertex shader");
    }
   }

   fshader = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(fshader,1,(const GLchar**)&fsource,0);
   glCompileShader(fshader);
   {
    int compiled = GL_TRUE;
    glGetShaderiv(fshader,GL_COMPILE_STATUS,&compiled);
    if ( compiled == GL_FALSE )
    {
     int maxLength = 0;
     glGetShaderiv(fshader,GL_INFO_LOG_LENGTH,&maxLength);
     char * Log = new char [ maxLength ];
     glGetShaderInfoLog(fshader,maxLength,&maxLength,Log);
     qDebug() << Log;
     delete Log;
     qFatal("can not compile fragment shader");
    }
   }

   program = glCreateProgram();
   glAttachShader(program,vshader);
   glAttachShader(program,fshader);

   glBindAttribLocation(program,0,"in_Position");
   glBindAttribLocation(program,1,"in_Color");

   glLinkProgram(program);

   {
    int linked = GL_TRUE;
    glGetProgramiv(program,GL_LINK_STATUS,&linked);
    if ( linked == GL_FALSE )
    {
     int maxLength = 0;
     glGetProgramiv(program,GL_INFO_LOG_LENGTH,&maxLength);
     char * Log = new char [ maxLength ];
     glGetProgramInfoLog(program,maxLength,&maxLength,Log);
     qDebug() << Log;
     delete Log;
     qFatal("can not link program");
    }
   }

   delete vsource;
   delete fsource;

 }

  void GridViewGL::resizeGL (int w, int h)
  {
   glViewport(0,0,w,h);
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
   glUseProgram(program);
   glDrawArrays(GL_POINTS,0,4);
   glUseProgram(0);
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
