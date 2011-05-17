#include "gridview.h"

namespace Langmuir
{

  GridViewGL::GridViewGL(QWidget * parent, QString input ):QGLWidget(parent)
  {
   setFocusPolicy(Qt::StrongFocus);
   pPar = new SimulationParameters;
   pInput = new InputParser(input);
   pInput->simulationParameters(pPar);
   if ( pPar->iterationsPrint > 100 ) { qFatal("iterations.print must be kept low for gridview to render"); }
   pSim = new Simulation(pPar);
   xRot  = 0;
   yRot  = 0;
   zRot  = 0;
   xTran = 0;
   yTran = 0;
   zTran = 0;
   xDelta = 2.5;
   yDelta = 2.5;
   zDelta = 100.0;
   thickness = 10.0;
   qtimer = new QTimer(this);
   connect(qtimer, SIGNAL(timeout()), this, SLOT(timerUpdateGL()));
  }

  GridViewGL::~GridViewGL()
  {
   delete pPar;
   delete pSim;
   delete pInput;
   delete carriers;
   delete defects;
   delete base;
   delete source;
   delete drain;
   delete side1;
   delete side2;
   delete side3;
   delete side4;
   delete side5;
   delete side6;
   delete x;
   delete y;
   delete z;
  }

  QSize GridViewGL::minimumSizeHint() const
  {
    return QSize(200, 200);
  }

  QSize GridViewGL::sizeHint() const
  {
    return QSize(pPar->gridWidth,pPar->gridHeight);
  }

  void GridViewGL::initializeGL()
  {
    glewInit();
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_POINT_SMOOTH);
    //glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);

    x = new Box( this,
                 QVector3D( 10*thickness, 1, 1 ),
                 QVector3D( -2.0*thickness, -2.0*thickness, -2.0*thickness ),
                 QColor( 255, 0, 0, 255 )
                 );
    y = new Box( this,
                 QVector3D( 1, 10.0*thickness, 1 ),
                 QVector3D( -2.0*thickness, -2.0*thickness, -2.0*thickness ),
                 QColor( 0, 255, 0, 255 )
               );
    z = new Box( this,
                 QVector3D( 1, 1, 10.0*thickness ),
                 QVector3D( -2.0*thickness, -2.0*thickness, -2.0*thickness ),
                 QColor( 0, 0, 255, 255 )
               );

    base = new Box( this,
                    QVector3D( pPar->gridWidth, pPar->gridHeight, thickness ),
                    QVector3D( 0, 0, -thickness ),
                    QColor( 25, 25, 25, 255 )
                  );
    source = new Box( this,
                      QVector3D( 2.0*thickness, pPar->gridHeight, thickness + pPar->gridDepth * thickness ),
                      QVector3D( -2.0*thickness, 0, -thickness ),
                      QColor( 0, 0, 255, 255 )
                    );
    drain = new Box( this,
                     QVector3D( 2.0*thickness, pPar->gridHeight, thickness + pPar->gridDepth * thickness ),
                     QVector3D( pPar->gridWidth, 0, -thickness ),
                     QColor( 255, 0, 0, 255 )
                   );

    side1 = new Box( this,
                     QVector3D( pPar->gridWidth, thickness, thickness ),
                     QVector3D( 0, -thickness, -thickness ),
                     QColor( 255, 255, 255, 255 )
                   );
    side2 = new Box( this,
                     QVector3D( pPar->gridWidth, thickness, thickness ),
                     QVector3D( 0, pPar->gridHeight, -thickness ),
                     QColor( 255, 255, 255, 255 )
                   );
    side3 = new Box( this,
                     QVector3D( 2.0*thickness, thickness, thickness + pPar->gridDepth * thickness ),
                     QVector3D( -2.0*thickness, -thickness, -thickness ),
                     QColor( 30, 144, 255, 255 )
                   );
    side4 = new Box( this,
                     QVector3D( 2.0*thickness, thickness, thickness + pPar->gridDepth * thickness ),
                     QVector3D( -2.0*thickness, pPar->gridHeight, -thickness ),
                     QColor( 30, 144, 255, 255 )
                   );
    side5 = new Box( this,
                     QVector3D( 2.0*thickness, thickness, thickness + pPar->gridDepth * thickness ),
                     QVector3D( pPar->gridWidth, -thickness, -thickness ),
                     QColor( 205, 92, 92, 255 )
                   );
    side6 = new Box( this,
                     QVector3D( 2.0*thickness, thickness, thickness + pPar->gridDepth * thickness ),
                     QVector3D( pPar->gridWidth, pPar->gridHeight, -thickness ),
                     QColor( 205, 92, 92, 255 )
                   );
     xRot  =  5000.0;
     yRot  =     0.0;
     zRot  =     0.0;

    QList< ChargeAgent* > *charges = pSim->world()->charges();
    QVector<float> xyz;
    Grid *grid = pSim->world()->grid();
    for ( int i = 0; i < charges->size(); i++ )
    {
     unsigned int site = charges->at(i)->site();
     Eigen::Vector3d position = grid->position( site );
     xyz.push_back( position.x() );
     xyz.push_back( position.y() );
     xyz.push_back( thickness*position.z() );
    }
    carriers = new PointArray(this,xyz,QColor(255,0,0,0),3.0f);

    QList< unsigned int > *defectList = pSim->world()->chargedDefects();
    xyz.clear();
    for ( int i = 0; i < defectList->size(); i++ )
    {
     Eigen::Vector3d position = grid->position( defectList->at(i) );
     xyz.push_back( position.x() );
     xyz.push_back( position.y() );
     xyz.push_back( thickness*position.z() );
    }
    defects  = new PointArray(this,xyz,QColor(0,255,0,255),3.0f);

    float move_camera = pPar->gridWidth;
    if ( pPar->gridWidth < pPar->gridHeight ) { move_camera = pPar->gridHeight; }
    xTran = -(pPar->gridWidth)*0.5f;
    yTran = -(pPar->gridHeight)*0.25f;
    zTran = -(move_camera)-0.01*(move_camera);

   qtimer->start(10);
  }

  void GridViewGL::timerUpdateGL()
  {
   pSim->performIterations( pPar->iterationsPrint );
   QList< ChargeAgent* > *charges = pSim->world()->charges();
   QVector<float> xyz;
   Grid *grid = pSim->world()->grid();
   for ( int i = 0; i < charges->size(); i++ )
   {
    unsigned int site = charges->at(i)->site();
    Eigen::Vector3d position = grid->position( site );
    xyz.push_back( position.x() );
    xyz.push_back( position.y() );
    xyz.push_back( thickness*position.z() );
   }
   carriers->update( xyz );
   updateGL();
   //qDebug() << QString("TRAN: (%1, %2, %3) ROTA: (%4, %5, %6)").arg(xTran).arg(yTran).arg(zTran).arg(xRot).arg(yRot).arg(zRot);
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
        setXTranslation(xTran+xDelta*dx);
        setYTranslation(yTran-yDelta*dy);
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
    gluPerspective(60.0f,(GLfloat)w/(GLfloat)h,0.01f,10000.0f);
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

    base->draw();
    source->draw();
    drain->draw();
    side1->draw();
    side2->draw();
    side3->draw();
    side4->draw();
    side5->draw();
    side6->draw();
    //x->draw();
    //y->draw();
    //z->draw();
    carriers->draw( pSim->world()->charges()->size() );
    defects->draw( pSim->world()->chargedDefects()->size() );
  }

  Box::Box( QObject *parent, QVector3D dimensions, QVector3D origin, QColor color ) : QObject(parent), col(color)
  {

   QVector<float> varray(72);
   QVector<float> narray(72);

   varray[ 0] = 1.0f; narray[ 0] =  0.0f;
   varray[ 1] = 1.0f; narray[ 1] =  1.0f;
   varray[ 2] = 0.0f; narray[ 2] =  0.0f;
   varray[ 3] = 0.0f; narray[ 3] =  0.0f;
   varray[ 4] = 1.0f; narray[ 4] =  1.0f;
   varray[ 5] = 0.0f; narray[ 5] =  0.0f;
   varray[ 6] = 0.0f; narray[ 6] =  0.0f;
   varray[ 7] = 1.0f; narray[ 7] =  1.0f;
   varray[ 8] = 1.0f; narray[ 8] =  0.0f;
   varray[ 9] = 1.0f; narray[ 9] =  0.0f;
   varray[10] = 1.0f; narray[10] =  1.0f;
   varray[11] = 1.0f; narray[11] =  0.0f;

   varray[12] = 1.0f; narray[12] =  0.0f;
   varray[13] = 0.0f; narray[13] = -1.0f;
   varray[14] = 1.0f; narray[14] =  0.0f;
   varray[15] = 0.0f; narray[15] =  0.0f;
   varray[16] = 0.0f; narray[16] = -1.0f;
   varray[17] = 1.0f; narray[17] =  0.0f;
   varray[18] = 0.0f; narray[18] =  0.0f;
   varray[19] = 0.0f; narray[19] = -1.0f;
   varray[20] = 0.0f; narray[20] =  0.0f;
   varray[21] = 1.0f; narray[21] =  0.0f;
   varray[22] = 0.0f; narray[22] = -1.0f;
   varray[23] = 0.0f; narray[23] =  0.0f;

   varray[24] = 1.0f; narray[24] =  0.0f;
   varray[25] = 1.0f; narray[25] =  0.0f;
   varray[26] = 1.0f; narray[26] =  1.0f;
   varray[27] = 0.0f; narray[27] =  0.0f;
   varray[28] = 1.0f; narray[28] =  0.0f;
   varray[29] = 1.0f; narray[29] =  1.0f;
   varray[30] = 0.0f; narray[30] =  0.0f;
   varray[31] = 0.0f; narray[31] =  0.0f;
   varray[32] = 1.0f; narray[32] =  1.0f;
   varray[33] = 1.0f; narray[33] =  0.0f;
   varray[34] = 0.0f; narray[34] =  0.0f;
   varray[35] = 1.0f; narray[35] =  1.0f;

   varray[36] = 1.0f; narray[36] =  0.0f;
   varray[37] = 0.0f; narray[37] =  0.0f;
   varray[38] = 0.0f; narray[38] = -1.0f;
   varray[39] = 0.0f; narray[39] =  0.0f;
   varray[40] = 0.0f; narray[40] =  0.0f;
   varray[41] = 0.0f; narray[41] = -1.0f;
   varray[42] = 0.0f; narray[42] =  0.0f;
   varray[43] = 1.0f; narray[43] =  0.0f;
   varray[44] = 0.0f; narray[44] = -1.0f;
   varray[45] = 1.0f; narray[45] =  0.0f;
   varray[46] = 1.0f; narray[46] =  0.0f;
   varray[47] = 0.0f; narray[47] = -1.0f;

   varray[48] = 0.0f; narray[48] = -1.0f;
   varray[49] = 1.0f; narray[49] =  0.0f;
   varray[50] = 1.0f; narray[50] =  0.0f;
   varray[51] = 0.0f; narray[51] = -1.0f;
   varray[52] = 1.0f; narray[52] =  0.0f;
   varray[53] = 0.0f; narray[53] =  0.0f;
   varray[54] = 0.0f; narray[54] = -1.0f;
   varray[55] = 0.0f; narray[55] =  0.0f;
   varray[56] = 0.0f; narray[56] =  0.0f;
   varray[57] = 0.0f; narray[57] = -1.0f;
   varray[58] = 0.0f; narray[58] =  0.0f;
   varray[59] = 1.0f; narray[59] =  0.0f;

   varray[60] = 1.0f; narray[60] =  1.0f;
   varray[61] = 1.0f; narray[61] =  0.0f;
   varray[62] = 0.0f; narray[62] =  0.0f;
   varray[63] = 1.0f; narray[63] =  1.0f;
   varray[64] = 1.0f; narray[64] =  0.0f;
   varray[65] = 1.0f; narray[65] =  0.0f;
   varray[66] = 1.0f; narray[66] =  1.0f;
   varray[67] = 0.0f; narray[67] =  0.0f;
   varray[68] = 1.0f; narray[68] =  0.0f;
   varray[69] = 1.0f; narray[69] =  1.0f;
   varray[70] = 0.0f; narray[70] =  0.0f;
   varray[71] = 0.0f; narray[71] =  0.0f;

   for ( int i = 0; i < varray.size(); i+=3 )
   {
    varray[i+0] = varray[i+0]*dimensions.x()+origin.x();
    varray[i+1] = varray[i+1]*dimensions.y()+origin.y();
    varray[i+2] = varray[i+2]*dimensions.z()+origin.z();
   }

   for ( int i = 0; i < varray.size(); i+=3 )
   {
    float nx = narray[i+0];
    float ny = narray[i+1];
    float nz = narray[i+2];
    float ss = 1.0f;
    for ( int j = 0; j < varray.size(); j+=12 )
    {
     if ( qFuzzyCompare( varray[i+0], varray[j+0] ) && qFuzzyCompare( varray[i+1], varray[j+1] ) && qFuzzyCompare( varray[i+2], varray[j+2] ) )
     {
      nx += narray[j+0];
      ny += narray[j+1];
      nz += narray[j+2];
      ss += 1;
     }
     else if ( qFuzzyCompare( varray[i+0], varray[j+3] ) && qFuzzyCompare( varray[i+1], varray[j+4] ) && qFuzzyCompare( varray[i+2], varray[j+5] ) )
     {
      nx += narray[j+0];
      ny += narray[j+1];
      nz += narray[j+2];
      ss += 1;
     }
     else if ( qFuzzyCompare( varray[i+0], varray[j+6] ) && qFuzzyCompare( varray[i+1], varray[j+7] ) && qFuzzyCompare( varray[i+2], varray[j+8] ) )
     {
      nx += narray[j+0];
      ny += narray[j+1];
      nz += narray[j+2];
      ss += 1;
     }
     else if ( qFuzzyCompare( varray[i+0], varray[j+9] ) && qFuzzyCompare( varray[i+1], varray[j+10] ) && qFuzzyCompare( varray[i+2], varray[j+11] ) )
     {
      nx += narray[j+0];
      ny += narray[j+1];
      nz += narray[j+2];
      ss += 1;
     }
    }
    narray[i+0] = nx / ss;
    narray[i+1] = ny / ss;
    narray[i+2] = nz / ss;
   }

   glGenBuffers(1,&vVBO);
   glBindBuffer(GL_ARRAY_BUFFER,vVBO);
   glBufferData(GL_ARRAY_BUFFER,varray.size()*sizeof(float),varray.constData(),GL_STATIC_DRAW);

   glGenBuffers(1,&nVBO);
   glBindBuffer(GL_ARRAY_BUFFER,nVBO);
   glBufferData(GL_ARRAY_BUFFER,narray.size()*sizeof(float),narray.constData(),GL_STATIC_DRAW);

  }

  Box::~Box()
  {
   glDeleteBuffers(1,&vVBO);
   glDeleteBuffers(1,&nVBO);
  }

  void Box::draw() const
  {
    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER,vVBO);
    glVertexPointer(3,GL_FLOAT,0,0);

    glEnableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER,nVBO);
    glNormalPointer(GL_FLOAT,0,0);

    glColor4f(col.redF(),col.greenF(),col.blueF(),col.alphaF());
    glDrawArrays(GL_QUADS,0,24);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
  }

  PointArray::PointArray( QObject *parent, QVector<float>& xyz, QColor color, float pointsize ) : QObject(parent), col(color)
  {
   m_pointsize = pointsize;
   glGenBuffers(1,&vVBO);
   glBindBuffer(GL_ARRAY_BUFFER,vVBO);
   glBufferData(GL_ARRAY_BUFFER,xyz.size()*sizeof(float),xyz.constData(),GL_DYNAMIC_DRAW);
  }

  PointArray::~PointArray()
  {
   glDeleteBuffers(1,&vVBO);
  }

  void PointArray::draw( int size ) const
  {
    glPointSize(m_pointsize);

    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER,vVBO);
    glVertexPointer(3,GL_FLOAT,0,0);

    glColor4f(col.redF(),col.greenF(),col.blueF(),col.alphaF());
    glDrawArrays(GL_POINTS,0,size);

    glDisableClientState(GL_VERTEX_ARRAY);
  }

  void PointArray::update( QVector<float>& xyz )
  {
   glBindBuffer(GL_ARRAY_BUFFER,vVBO);
   glBufferData(GL_ARRAY_BUFFER,xyz.size()*sizeof(float),xyz.constData(),GL_DYNAMIC_DRAW);
  }

  MainWindow::MainWindow(QString input)
  {
    glWidget = new GridViewGL(this,input);
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(glWidget);
    setLayout(mainLayout);
    setWindowTitle(tr("Langmuir View"));
  }

}
