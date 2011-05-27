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
   translation.setX( 0 );
   translation.setY( 0 );
   translation.setZ( 0 );
   rotation.setX( 0 );
   rotation.setY( 0 );
   rotation.setZ( 0 );
   delta.setX( 5.0 );
   delta.setY( 5.0 );
   delta.setZ( 25.0 );
   thickness = 10.0;
   fov = 60.0;
   pause = true;
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
   delete background;
  }

  QSize GridViewGL::minimumSizeHint() const
  {
    return QSize(50, 50);
  }

  QSize GridViewGL::sizeHint() const
  {
    return QSize(pPar->gridWidth + 4 * thickness,pPar->gridHeight + 2 * thickness);
  }

  void GridViewGL::initializeGL()
  {
    glewInit();
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);

    float light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    light[0] = 1.0f; light[1] = 1.0f; light[2] = 1.0f; light[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, light );
    light[0] = 0.0f; light[1] = 0.0f; light[2] = 0.0f; light[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_EMISSION, light );
    glMaterialf(GL_FRONT, GL_SHININESS, 128);
    glEnable(GL_LIGHT0);

    base = new Box( this,
                    QVector3D( pPar->gridWidth, pPar->gridHeight, thickness ),
                    QVector3D( 0, 0, -thickness ),
                    QColor( 247, 216, 9, 255 )
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
                     QColor( 255, 255, 255, 255 )
                   );
    side4 = new Box( this,
                     QVector3D( 2.0*thickness, thickness, thickness + pPar->gridDepth * thickness ),
                     QVector3D( -2.0*thickness, pPar->gridHeight, -thickness ),
                     QColor( 255, 255, 255, 255 )
                   );
    side5 = new Box( this,
                     QVector3D( 2.0*thickness, thickness, thickness + pPar->gridDepth * thickness ),
                     QVector3D( pPar->gridWidth, -thickness, -thickness ),
                     QColor( 255, 255, 255, 255 )
                   );
    side6 = new Box( this,
                     QVector3D( 2.0*thickness, thickness, thickness + pPar->gridDepth * thickness ),
                     QVector3D( pPar->gridWidth, pPar->gridHeight, -thickness ),
                     QColor( 255, 255, 255, 255 )
                   );

    background = new ColoredObject( this );
    background->setColor( QColor( 28, 164, 255, 0 ) );

    Grid *grid = pSim->world()->grid();

    pointBuffer.resize( (int(pPar->chargePercentage * grid->volume())) * 4 );
    for ( int i = 0; i < pointBuffer.size(); i+= 4 )
    {
     pointBuffer[i]   = 0;
     pointBuffer[i+1] = 0;
     pointBuffer[i+2] = 0;
     pointBuffer[i+3] = 1;
    }

    QList< ChargeAgent* > *charges = pSim->world()->charges();
    for ( int i = 0; i < charges->size(); i++ )
    {
     unsigned int site = charges->at(i)->site();
     Eigen::Vector3d position = grid->position( site );
     pointBuffer[ i * 4 + 0 ] = position.x();
     pointBuffer[ i * 4 + 1 ] = position.y();
     pointBuffer[ i * 4 + 2 ] = position.z() * thickness;
    }
    carriers = new PointArray(this,pointBuffer,QColor(255,0,0,0),2.5f);

    QList< unsigned int > *defectList = pSim->world()->chargedDefects();
    QVector< float > xyz( defectList->size() * 4, 1 );
    for ( int i = 0; i < defectList->size(); i++ )
    {
     Eigen::Vector3d position = grid->position( defectList->at(i) );
     xyz[ i * 4 + 0 ] = position.x();
     xyz[ i * 4 + 1 ] = position.y();
     xyz[ i * 4 + 2 ] = position.z() * thickness;
    }
    defects  = new PointArray(this,xyz,QColor(0,0,0,255),2.5f);

   translation.setX( -(pPar->gridWidth)*0.5f );
   translation.setY( -(pPar->gridHeight)*0.5f );
   translation.setZ( -(pPar->gridHeight)-thickness );
   rotation.setX( 0 );
   rotation.setY( 0 );
   rotation.setZ( 0 );

   initializeNavigator();
   initializeSceneOptions();
   qtimer->start(10);
  }

  void GridViewGL::timerUpdateGL()
  {
   if ( ! pause )
   {
   pSim->performIterations( pPar->iterationsPrint );

   QList< ChargeAgent* > *charges = pSim->world()->charges();
   Grid *grid = pSim->world()->grid();
   for ( int i = 0; i < charges->size(); i++ )
   {
    unsigned int site = charges->at(i)->site();
    Eigen::Vector3d position = grid->position( site );
    pointBuffer[ i * 4 + 0 ] = position.x();
    pointBuffer[ i * 4 + 1 ] = position.y();
    pointBuffer[ i * 4 + 2 ] = position.z() * thickness;
   }
   carriers->update( pointBuffer, charges->size() );
   }
   updateGL();
  }

  void GridViewGL::NormalizeAngle(double &angle)
  {
    while(angle < 0)
      angle += 360;
    while(angle > 360)
      angle -= 360;
  }

  void GridViewGL::setXRotation(double angle)
  {
    NormalizeAngle(angle);
    if ( angle != rotation.x() )
    {
     rotation.setX(angle);
     emit xRotationChanged( angle );
     updateGL();
    }
  }

  void GridViewGL::setYRotation(double angle)
  {
    NormalizeAngle(angle);
    if ( angle != rotation.y() )
    {
     rotation.setY(angle);
     emit yRotationChanged( angle );
     updateGL();
    }
  }

  void GridViewGL::setZRotation(double angle)
  {
    NormalizeAngle(angle);
    if ( angle != rotation.z() )
    {
     rotation.setZ(angle);
     emit zRotationChanged( angle );
     updateGL();
    }
  }

  void GridViewGL::setXTranslation(double length)
  {
   if ( length != translation.x() )
   {
     translation.setX(length);
     emit xTranslationChanged( length );
     updateGL();
   }
  }

  void GridViewGL::setYTranslation(double length)
  {
   if ( length != translation.y() )
   {
     translation.setY(length);
     emit yTranslationChanged( length );
     updateGL();
   }
  }

  void GridViewGL::setZTranslation(double length)
  {
   if ( length != translation.z() )
   {
     translation.setZ(length);
     emit zTranslationChanged( length );
     updateGL();
   }
  }

  void GridViewGL::setPauseStatus()
  {
   pause = !( pause );
   if ( pause ) { emit pauseChanged( QString("Resume") ); }
   else { emit pauseChanged( QString("Pause") ); }
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
        setXRotation(rotation.x() + ( 8 * dy ) / 16.0 );
        setYRotation(rotation.y() + ( 8 * dx ) / 16.0 );
      }
    else if(event->buttons() & Qt::LeftButton)
      {
        setXTranslation(translation.x() + delta.x() * dx );
        setYTranslation(translation.y() - delta.y() * dy );
      }
    lastPos = event->pos();
  }

  void GridViewGL::wheelEvent(QWheelEvent *event)
  {
   setZTranslation(translation.z() + delta.z() * event->delta()/120.0);
  }

  void GridViewGL::keyPressEvent(QKeyEvent *event)
  {
   switch ( event->key() )
   {
    case Qt::Key_Left:
    {
     setXTranslation(translation.x() - delta.x() );
     break;
    }
    case Qt::Key_Right:
    {
     setXTranslation(translation.x() + delta.x() );
     break;
    }
    case Qt::Key_Up:
    {
     setYTranslation(translation.y() + delta.y() );
     break;
    }
    case Qt::Key_Down:
    {
     setYTranslation(translation.y() - delta.y() );
     break;
    }
    case Qt::Key_Plus:
    {
     setZTranslation(translation.z() + delta.z() );
     break;
    }
    case Qt::Key_Minus:
    {
     setZTranslation(translation.z() - delta.z() );
     break;
    }
    case Qt::Key_P:
    {
     setPauseStatus();
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
    gluPerspective(fov,(GLfloat)w/(GLfloat)h,0.01f,10000.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  }

  void GridViewGL::paintGL()
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(background->getColor().redF(),background->getColor().greenF(),background->getColor().blueF(),background->getColor().alphaF());
    glLoadIdentity();
    glTranslatef(translation.x(),translation.y(),translation.z());
    glRotatef(rotation.x(),1.0,0.0,0.0);
    glRotatef(rotation.y(),0.0,1.0,0.0);
    glRotatef(rotation.z(),0.0,0.0,1.0);

    base->draw();
    source->draw();
    drain->draw();
    side1->draw();
    side2->draw();
    side3->draw();
    side4->draw();
    side5->draw();
    side6->draw();
    carriers->draw( pSim->world()->charges()->size(), this->height(), fov );
    defects->draw( pSim->world()->chargedDefects()->size(), this->height(), fov );
  }

  Box::Box( QObject *parent, QVector3D dimensions, QVector3D origin, QColor color ) : ColoredObject(parent)
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

   this->color = color;
  }

  Box::~Box()
  {
   glDeleteBuffers(1,&vVBO);
   glDeleteBuffers(1,&nVBO);
  }

  void Box::draw() const
  {
    if ( ! invisible )
    {
     glEnable(GL_LIGHTING);

     glEnableClientState(GL_VERTEX_ARRAY);
     glBindBuffer(GL_ARRAY_BUFFER,vVBO);
     glVertexPointer(3,GL_FLOAT,0,0);

     glEnableClientState(GL_NORMAL_ARRAY);
     glBindBuffer(GL_ARRAY_BUFFER,nVBO);
     glNormalPointer(GL_FLOAT,0,0);

     float light[] = { color.redF(), color.greenF(), color.blueF(), color.alphaF() };
     glMaterialfv(GL_FRONT, GL_AMBIENT, light );
     glMaterialfv(GL_FRONT, GL_DIFFUSE, light );

     glDrawArrays(GL_QUADS,0,24);

     glDisableClientState(GL_VERTEX_ARRAY);
     glDisableClientState(GL_NORMAL_ARRAY);

     glDisable(GL_LIGHTING);
    }
  }

  PointArray::PointArray( QObject *parent, QVector<float>& xyz, QColor color, float pointsize ) : ColoredObject(parent), vVBO(0), prog(0)
  {
   const char *vsource =
   " uniform float pointRadius;  // point size in world space\n"
   " uniform float pointScale;   // scale to calculate size in pixels\n"
   " uniform float densityScale;\n"
   " uniform float densityOffset;\n"
   " varying vec3 posEye;        // position of center in eye space\n"
   " void main()\n"
   " {\n"
   "     // calculate window-space point size\n"
   "     posEye = vec3(gl_ModelViewMatrix * vec4(gl_Vertex.xyz, 1.0));\n"
   "     float dist = length(posEye);\n"
   "     gl_PointSize = pointRadius * (pointScale / dist);\n"
   "\n"
   "     gl_TexCoord[0] = gl_MultiTexCoord0;\n"
   "     gl_Position = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz, 1.0);\n"
   "\n"
   "     gl_FrontColor = gl_Color;\n"
   " }\n";

   const char *fsource =
   " uniform float pointRadius;  // point size in world space\n"
   " varying vec3 posEye;        // position of center in eye space\n"
   " void main()\n"
   " {\n"
   "     const vec3 lightDir = vec3(0.577, 0.577, 0.577);\n"
   "     const float shininess = 40.0;\n"
   "\n"
   "     // calculate normal from texture coordinates\n"
   "     vec3 n;\n"
   "     n.xy = gl_TexCoord[0].xy*vec2(2.0, -2.0) + vec2(-1.0, 1.0);\n"
   "     float mag = dot(n.xy, n.xy);\n"
   "     if (mag > 1.0) discard;   // kill pixels outside circle\n"
   "     n.z = sqrt(1.0-mag);\n"
   "\n"
   "     // point on surface of sphere in eye space\n"
   "     vec3 spherePosEye = posEye + n*pointRadius;\n"
   "\n"
   "     // calculate lighting\n"
   "     float diffuse = max(0.0, dot(lightDir, n));\n"
   "     vec3 v = normalize(-spherePosEye);\n"
   "     vec3 h = normalize(lightDir + v);\n"
   "     float specular = pow(max(0.0, dot(n, h)), shininess);\n"
   "\n"
   " //    gl_FragColor = gl_Color * diffuse;\n"
   "     gl_FragColor = gl_Color * diffuse + specular;\n"
   " }\n";

   GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
   GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(vertexShader, 1, &vsource, 0);
   glShaderSource(fragmentShader, 1, &fsource, 0);
   glCompileShader(vertexShader);
   glCompileShader(fragmentShader);
   prog = glCreateProgram();
   glAttachShader(prog, vertexShader);
   glAttachShader(prog, fragmentShader);
   glLinkProgram(prog);
   GLint success = 0;
   glGetProgramiv(prog, GL_LINK_STATUS, &success);
   if (!success) {
       char temp[256];
       glGetProgramInfoLog(prog, 256, 0, temp);
       qDebug("Failed to link program:\n%s\n", temp);
       glDeleteProgram(prog);
       prog = 0;
   }
   #if !defined(__APPLE__) && !defined(MACOSX)
    glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);
    glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);
   #endif

   this->pointSize = pointsize;
   this->color = color;
   this->spheres = true;

   glGenBuffers(1,&vVBO);
   glBindBuffer(GL_ARRAY_BUFFER,vVBO);
   glBufferData(GL_ARRAY_BUFFER,xyz.size()*sizeof(float),xyz.constData(),GL_DYNAMIC_DRAW);
  }

  PointArray::~PointArray()
  {
   glDeleteProgram(prog);
   glDeleteBuffers(1,&vVBO);
  }

  void PointArray::setSpheres( int checkState )
  { 
   switch ( checkState )
   {
    case Qt::Checked :
     this->spheres = true; 
     break;
    case Qt::Unchecked :
     this->spheres = false; 
     break;
    case Qt::PartiallyChecked :
     this->spheres = false; 
     break;
    default :
     this->spheres = false; 
     break;
   }
  }

  void PointArray::setPointSize( double pointSize )
  {
    this->pointSize = pointSize;
    emit pointSizeChanged( this->pointSize );
  }

  void PointArray::draw( int size, int height, float fov ) const
  {
   if ( ! invisible )
   {
    if ( spheres )
    {
     glEnable(GL_POINT_SPRITE);
     glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
     glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
     glDepthMask(GL_TRUE);
     glEnable(GL_DEPTH_TEST);

     glUseProgram(prog);
     glUniform1f( glGetUniformLocation(prog, "pointScale"),  height / tanf( fov * 0.5 * M_PI / 180.0 ) );
     glUniform1f( glGetUniformLocation(prog, "pointRadius"), pointSize );

     glColor4f(color.redF(),color.greenF(),color.blueF(),color.alphaF());
     glBindBuffer(GL_ARRAY_BUFFER, vVBO);
     glVertexPointer(4, GL_FLOAT, 0, 0);
     glEnableClientState(GL_VERTEX_ARRAY);
     glDrawArrays(GL_POINTS, 0, size);
     glBindBuffer(GL_ARRAY_BUFFER, 0);
     glDisableClientState(GL_VERTEX_ARRAY);
     glDisableClientState(GL_COLOR_ARRAY);

     glUseProgram(0);
     glDisable(GL_POINT_SPRITE);
    }
    else
    {
     glPointSize(pointSize);
     glEnable(GL_POINT_SMOOTH);
     glColor4f(color.redF(),color.greenF(),color.blueF(),color.alphaF());
     glBindBuffer(GL_ARRAY_BUFFER,vVBO);
     glVertexPointer(4,GL_FLOAT,0,0);
     glEnableClientState(GL_VERTEX_ARRAY);
     glDrawArrays(GL_POINTS,0,size);
     glBindBuffer(GL_ARRAY_BUFFER, 0);
     glDisableClientState(GL_VERTEX_ARRAY);
     glDisableClientState(GL_COLOR_ARRAY);
     glDisable(GL_POINT_SMOOTH);
    }
   }
  }

  void PointArray::update( QVector<float>& xyz, int size )
  {
   glBindBuffer(GL_ARRAY_BUFFER,vVBO);
   glBufferData(GL_ARRAY_BUFFER,size*4*sizeof(float),xyz.constData(),GL_DYNAMIC_DRAW);
  }

  MainWindow::MainWindow(QString input)
  {
    glWidget = new GridViewGL(this,input);
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(glWidget);
    setLayout(mainLayout);
    setWindowTitle(tr("Langmuir View"));
  }

  void GridViewGL::initializeNavigator()
  {
   navigator = new StayOpen( this );
   navigator->setWindowFlags( Qt::Window );
   navigator->setWindowTitle("navigator");
   navigator->resize( 355, 107 );

   navigatorLabels.push_back( new QLabel( navigator ) );
   navigatorLabels.push_back( new QLabel( navigator ) );
   navigatorLabels.push_back( new QLabel( navigator ) );
   navigatorLabels.push_back( new QLabel( navigator ) );
   navigatorLabels.push_back( new QLabel( navigator ) );
   navigatorLabels.push_back( new QLabel( navigator ) );
   navigatorLabels[0]->setText("x");
   navigatorLabels[1]->setText("y");
   navigatorLabels[2]->setText("z");
   navigatorLabels[3]->setText("a");
   navigatorLabels[4]->setText("b");
   navigatorLabels[5]->setText("c");

   navigatorDSBoxes.push_back( new SpinBox( navigator ) );
   navigatorDSBoxes.push_back( new SpinBox( navigator ) );
   navigatorDSBoxes.push_back( new SpinBox( navigator ) );
   navigatorDSBoxes.push_back( new SpinBox( navigator ) );
   navigatorDSBoxes.push_back( new SpinBox( navigator ) );
   navigatorDSBoxes.push_back( new SpinBox( navigator ) );

   navigatorDSBoxes[0]->setDecimals(2);
   navigatorDSBoxes[1]->setDecimals(2);
   navigatorDSBoxes[2]->setDecimals(2);
   navigatorDSBoxes[3]->setDecimals(2);
   navigatorDSBoxes[4]->setDecimals(2);
   navigatorDSBoxes[5]->setDecimals(2);

   navigatorDSBoxes[0]->setMinimum(-10000);
   navigatorDSBoxes[1]->setMinimum(-10000);
   navigatorDSBoxes[2]->setMinimum(-10000);
   navigatorDSBoxes[3]->setMinimum(0);
   navigatorDSBoxes[4]->setMinimum(0);
   navigatorDSBoxes[5]->setMinimum(0);

   navigatorDSBoxes[0]->setMaximum(10000);
   navigatorDSBoxes[1]->setMaximum(10000);
   navigatorDSBoxes[2]->setMaximum(10000);
   navigatorDSBoxes[3]->setMaximum(360);
   navigatorDSBoxes[4]->setMaximum(360);
   navigatorDSBoxes[5]->setMaximum(360);

   navigatorDSBoxes[0]->setSingleStep(delta.x());
   navigatorDSBoxes[1]->setSingleStep(delta.y());
   navigatorDSBoxes[2]->setSingleStep(delta.z());
   navigatorDSBoxes[3]->setSingleStep(1.0);
   navigatorDSBoxes[4]->setSingleStep(1.0);
   navigatorDSBoxes[5]->setSingleStep(1.0);

   navigatorDSBoxes[0]->setValue(translation.x());
   navigatorDSBoxes[1]->setValue(translation.y());
   navigatorDSBoxes[2]->setValue(translation.z());
   navigatorDSBoxes[3]->setValue(rotation.x());
   navigatorDSBoxes[4]->setValue(rotation.y());
   navigatorDSBoxes[5]->setValue(rotation.z());

   navigatorDSBoxes[3]->setWrapping(true);
   navigatorDSBoxes[4]->setWrapping(true);
   navigatorDSBoxes[5]->setWrapping(true);

   navigatorPauseButton = new Button( navigator );
   navigatorPauseButton->setText("Start");

   navigatorLayout = new QGridLayout( navigator );
   navigatorLayout->addWidget(  navigatorLabels[0],  0,  0 );
   navigatorLayout->addWidget(  navigatorLabels[1],  0,  2 );
   navigatorLayout->addWidget(  navigatorLabels[2],  0,  4 );
   navigatorLayout->addWidget(  navigatorLabels[3],  1,  0 );
   navigatorLayout->addWidget(  navigatorLabels[4],  1,  2 );
   navigatorLayout->addWidget(  navigatorLabels[5],  1,  4 );
   navigatorLayout->addWidget( navigatorDSBoxes[0],  0,  1 );
   navigatorLayout->addWidget( navigatorDSBoxes[1],  0,  3 );
   navigatorLayout->addWidget( navigatorDSBoxes[2],  0,  5 );
   navigatorLayout->addWidget( navigatorDSBoxes[3],  1,  1 );
   navigatorLayout->addWidget( navigatorDSBoxes[4],  1,  3 );
   navigatorLayout->addWidget( navigatorDSBoxes[5],  1,  5 );
   navigatorLayout->addWidget( navigatorPauseButton,  2,  0, 6, 6 );

   connect( navigatorDSBoxes[0], SIGNAL( valueChanged(double) ), this, SLOT( setXTranslation(double) ) );
   connect( navigatorDSBoxes[1], SIGNAL( valueChanged(double) ), this, SLOT( setYTranslation(double) ) );
   connect( navigatorDSBoxes[2], SIGNAL( valueChanged(double) ), this, SLOT( setZTranslation(double) ) );

   connect( navigatorDSBoxes[3], SIGNAL( valueChanged(double) ), this, SLOT( setXRotation(double) ) );
   connect( navigatorDSBoxes[4], SIGNAL( valueChanged(double) ), this, SLOT( setYRotation(double) ) );
   connect( navigatorDSBoxes[5], SIGNAL( valueChanged(double) ), this, SLOT( setZRotation(double) ) );

   connect( this, SIGNAL( xTranslationChanged(double) ), navigatorDSBoxes[0], SLOT( setValueSlot(double) ) );
   connect( this, SIGNAL( yTranslationChanged(double) ), navigatorDSBoxes[1], SLOT( setValueSlot(double) ) );
   connect( this, SIGNAL( zTranslationChanged(double) ), navigatorDSBoxes[2], SLOT( setValueSlot(double) ) );

   connect( this, SIGNAL( xRotationChanged(double) ), navigatorDSBoxes[3], SLOT( setValueSlot(double) ) );
   connect( this, SIGNAL( yRotationChanged(double) ), navigatorDSBoxes[4], SLOT( setValueSlot(double) ) );
   connect( this, SIGNAL( zRotationChanged(double) ), navigatorDSBoxes[5], SLOT( setValueSlot(double) ) );

   connect( navigatorPauseButton, SIGNAL( clicked() ), this, SLOT( setPauseStatus() ) );
   connect( this, SIGNAL( pauseChanged( QString ) ), navigatorPauseButton, SLOT( setTextSlot( QString ) ) );

   navigator->show();

  }

  void GridViewGL::initializeSceneOptions()
  {
   sceneOptions = new StayOpen( this );
   sceneOptions->setWindowFlags( Qt::Window );
   sceneOptions->setWindowTitle("scene options");
   sceneOptions->resize( 448, 247 );

   sceneOptionsColorDialog = new QColorDialog( sceneOptions );

   sceneOptionsLabels.push_back( new QLabel( sceneOptions ) );
   sceneOptionsLabels.push_back( new QLabel( sceneOptions ) );
   sceneOptionsLabels.push_back( new QLabel( sceneOptions ) );
   sceneOptionsLabels.push_back( new QLabel( sceneOptions ) );
   sceneOptionsLabels.push_back( new QLabel( sceneOptions ) );
   sceneOptionsLabels.push_back( new QLabel( sceneOptions ) );
   sceneOptionsLabels.push_back( new QLabel( sceneOptions ) );
   sceneOptionsLabels.push_back( new QLabel( sceneOptions ) );
   sceneOptionsLabels.push_back( new QLabel( sceneOptions ) );

   sceneOptionsLabels[0]->setText("carriers");
   sceneOptionsLabels[1]->setText("defects");
   sceneOptionsLabels[2]->setText("source");
   sceneOptionsLabels[3]->setText("drain");
   sceneOptionsLabels[4]->setText("base");
   sceneOptionsLabels[5]->setText("background");
   sceneOptionsLabels[6]->setText("sides");
   sceneOptionsLabels[7]->setText("size");
   sceneOptionsLabels[8]->setText("size");

   sceneOptionsLabels[0]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   sceneOptionsLabels[1]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   sceneOptionsLabels[2]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   sceneOptionsLabels[3]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   sceneOptionsLabels[4]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   sceneOptionsLabels[5]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   sceneOptionsLabels[6]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   sceneOptionsLabels[7]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   sceneOptionsLabels[8]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

   sceneOptionsColorButtons.push_back( new ColorButton( sceneOptions, carriers, sceneOptionsColorDialog ) );
   sceneOptionsColorButtons.push_back( new ColorButton( sceneOptions, defects, sceneOptionsColorDialog ) );
   sceneOptionsColorButtons.push_back( new ColorButton( sceneOptions, source, sceneOptionsColorDialog ) );
   sceneOptionsColorButtons.push_back( new ColorButton( sceneOptions, drain, sceneOptionsColorDialog ) );
   sceneOptionsColorButtons.push_back( new ColorButton( sceneOptions, base, sceneOptionsColorDialog ) );
   sceneOptionsColorButtons.push_back( new ColorButton( sceneOptions, background, sceneOptionsColorDialog ) );
   sceneOptionsColorButtons.push_back( new ColorButton( sceneOptions, side1, sceneOptionsColorDialog ) );

   sceneOptionsColorButtons[0]->setText( "set color" );
   sceneOptionsColorButtons[1]->setText( "set color" );
   sceneOptionsColorButtons[2]->setText( "set color" );
   sceneOptionsColorButtons[3]->setText( "set color" );
   sceneOptionsColorButtons[4]->setText( "set color" );
   sceneOptionsColorButtons[5]->setText( "set color" );
   sceneOptionsColorButtons[6]->setText( "set color" );

   sceneOptionsCheckBoxes.push_back( new QCheckBox( sceneOptions ) );
   sceneOptionsCheckBoxes.push_back( new QCheckBox( sceneOptions ) );
   sceneOptionsCheckBoxes.push_back( new QCheckBox( sceneOptions ) );
   sceneOptionsCheckBoxes.push_back( new QCheckBox( sceneOptions ) );
   sceneOptionsCheckBoxes.push_back( new QCheckBox( sceneOptions ) );
   sceneOptionsCheckBoxes.push_back( new QCheckBox( sceneOptions ) );
   sceneOptionsCheckBoxes.push_back( new QCheckBox( sceneOptions ) );
   sceneOptionsCheckBoxes.push_back( new QCheckBox( sceneOptions ) );

   sceneOptionsCheckBoxes[0]->setText("hide");
   sceneOptionsCheckBoxes[1]->setText("hide");
   sceneOptionsCheckBoxes[2]->setText("hide");
   sceneOptionsCheckBoxes[3]->setText("hide");
   sceneOptionsCheckBoxes[4]->setText("hide");
   sceneOptionsCheckBoxes[5]->setText("hide");
   sceneOptionsCheckBoxes[6]->setText("spheres");
   sceneOptionsCheckBoxes[7]->setText("spheres");

   sceneOptionsCheckBoxes[6]->setChecked(Qt::Checked);
   sceneOptionsCheckBoxes[7]->setChecked(Qt::Checked);

   sceneOptionsDSBoxes.push_back( new SpinBox( navigator ) );
   sceneOptionsDSBoxes.push_back( new SpinBox( navigator ) );

   sceneOptionsDSBoxes[0]->setDecimals(2);
   sceneOptionsDSBoxes[1]->setDecimals(2);

   sceneOptionsDSBoxes[0]->setMinimum(0);
   sceneOptionsDSBoxes[1]->setMinimum(0);

   sceneOptionsDSBoxes[0]->setMaximum(100.0);
   sceneOptionsDSBoxes[1]->setMaximum(100.0);

   sceneOptionsDSBoxes[0]->setSingleStep(0.1);
   sceneOptionsDSBoxes[1]->setSingleStep(0.1);

   sceneOptionsDSBoxes[0]->setValue(2.5);
   sceneOptionsDSBoxes[1]->setValue(2.5);

   sceneOptionsDSBoxes[0]->setWrapping(true);
   sceneOptionsDSBoxes[1]->setWrapping(true);

   sceneOptionsLayout = new QGridLayout( sceneOptions );
   sceneOptionsLayout->addWidget(       sceneOptionsLabels[0],  0,  0 );
   sceneOptionsLayout->addWidget(       sceneOptionsLabels[1],  1,  0 );
   sceneOptionsLayout->addWidget(       sceneOptionsLabels[2],  2,  0 );
   sceneOptionsLayout->addWidget(       sceneOptionsLabels[3],  3,  0 );
   sceneOptionsLayout->addWidget(       sceneOptionsLabels[4],  4,  0 );
   sceneOptionsLayout->addWidget(       sceneOptionsLabels[5],  6,  0 );
   sceneOptionsLayout->addWidget(       sceneOptionsLabels[6],  5,  0 );
   sceneOptionsLayout->addWidget(       sceneOptionsLabels[7],  0,  4 );
   sceneOptionsLayout->addWidget(       sceneOptionsLabels[8],  1,  4 );
   sceneOptionsLayout->addWidget( sceneOptionsColorButtons[0],  0,  1 );
   sceneOptionsLayout->addWidget( sceneOptionsColorButtons[1],  1,  1 );
   sceneOptionsLayout->addWidget( sceneOptionsColorButtons[2],  2,  1 );
   sceneOptionsLayout->addWidget( sceneOptionsColorButtons[3],  3,  1 );
   sceneOptionsLayout->addWidget( sceneOptionsColorButtons[4],  4,  1 );
   sceneOptionsLayout->addWidget( sceneOptionsColorButtons[6],  5,  1 );
   sceneOptionsLayout->addWidget( sceneOptionsColorButtons[5],  6,  1 );
   sceneOptionsLayout->addWidget(   sceneOptionsCheckBoxes[0],  0,  2 );
   sceneOptionsLayout->addWidget(   sceneOptionsCheckBoxes[1],  1,  2 );
   sceneOptionsLayout->addWidget(   sceneOptionsCheckBoxes[2],  2,  2 );
   sceneOptionsLayout->addWidget(   sceneOptionsCheckBoxes[3],  3,  2 );
   sceneOptionsLayout->addWidget(   sceneOptionsCheckBoxes[4],  4,  2 );
   sceneOptionsLayout->addWidget(   sceneOptionsCheckBoxes[5],  5,  2 );
   sceneOptionsLayout->addWidget(   sceneOptionsCheckBoxes[6],  0,  3 );
   sceneOptionsLayout->addWidget(   sceneOptionsCheckBoxes[7],  1,  3 );
   sceneOptionsLayout->addWidget(      sceneOptionsDSBoxes[0],  0,  5 );
   sceneOptionsLayout->addWidget(      sceneOptionsDSBoxes[1],  1,  5 );

   connect( sceneOptionsColorButtons[0], SIGNAL( clicked() ), sceneOptionsColorButtons[0], SLOT( openColorDialog() ) );
   connect( sceneOptionsColorButtons[1], SIGNAL( clicked() ), sceneOptionsColorButtons[1], SLOT( openColorDialog() ) );
   connect( sceneOptionsColorButtons[2], SIGNAL( clicked() ), sceneOptionsColorButtons[2], SLOT( openColorDialog() ) );
   connect( sceneOptionsColorButtons[3], SIGNAL( clicked() ), sceneOptionsColorButtons[3], SLOT( openColorDialog() ) );
   connect( sceneOptionsColorButtons[4], SIGNAL( clicked() ), sceneOptionsColorButtons[4], SLOT( openColorDialog() ) );
   connect( sceneOptionsColorButtons[5], SIGNAL( clicked() ), sceneOptionsColorButtons[5], SLOT( openColorDialog() ) );
   connect( sceneOptionsColorButtons[6], SIGNAL( clicked() ), sceneOptionsColorButtons[6], SLOT( openColorDialog() ) );
   connect( side1, SIGNAL( colorChanged( QColor ) ), side2, SLOT( setColor( QColor ) ) );
   connect( side1, SIGNAL( colorChanged( QColor ) ), side3, SLOT( setColor( QColor ) ) );
   connect( side1, SIGNAL( colorChanged( QColor ) ), side4, SLOT( setColor( QColor ) ) );
   connect( side1, SIGNAL( colorChanged( QColor ) ), side5, SLOT( setColor( QColor ) ) );
   connect( side1, SIGNAL( colorChanged( QColor ) ), side6, SLOT( setColor( QColor ) ) );

   connect( sceneOptionsCheckBoxes[0], SIGNAL( stateChanged(int) ), carriers, SLOT( setInvisible(int) ) );
   connect( sceneOptionsCheckBoxes[1], SIGNAL( stateChanged(int) ), defects, SLOT( setInvisible(int) ) );
   connect( sceneOptionsCheckBoxes[2], SIGNAL( stateChanged(int) ), source, SLOT( setInvisible(int) ) );
   connect( sceneOptionsCheckBoxes[3], SIGNAL( stateChanged(int) ), drain, SLOT( setInvisible(int) ) );
   connect( sceneOptionsCheckBoxes[4], SIGNAL( stateChanged(int) ), base, SLOT( setInvisible(int) ) );
   connect( sceneOptionsCheckBoxes[5], SIGNAL( stateChanged(int) ), side1, SLOT( setInvisible(int) ) );
   connect( sceneOptionsCheckBoxes[5], SIGNAL( stateChanged(int) ), side2, SLOT( setInvisible(int) ) );
   connect( sceneOptionsCheckBoxes[5], SIGNAL( stateChanged(int) ), side3, SLOT( setInvisible(int) ) );
   connect( sceneOptionsCheckBoxes[5], SIGNAL( stateChanged(int) ), side4, SLOT( setInvisible(int) ) );
   connect( sceneOptionsCheckBoxes[5], SIGNAL( stateChanged(int) ), side5, SLOT( setInvisible(int) ) );
   connect( sceneOptionsCheckBoxes[5], SIGNAL( stateChanged(int) ), side6, SLOT( setInvisible(int) ) );
   connect( sceneOptionsCheckBoxes[6], SIGNAL( stateChanged(int) ), carriers, SLOT( setSpheres(int) ) );
   connect( sceneOptionsCheckBoxes[7], SIGNAL( stateChanged(int) ), defects, SLOT( setSpheres(int) ) );

   connect( sceneOptionsDSBoxes[0], SIGNAL( valueChanged(double) ), carriers, SLOT( setPointSize(double) ) );
   connect( sceneOptionsDSBoxes[1], SIGNAL( valueChanged(double) ), defects, SLOT( setPointSize(double) ) );   

   sceneOptions->show();
  }

  void ColorButton::openColorDialog()
  { 
   if ( coloredObject && colorDialog )
   { 
     colorDialog->setCurrentColor( coloredObject->getColor() );
     colorDialog->open( coloredObject, SLOT( setColor( QColor ) ) );
   }
   else
   {
     qDebug() << "can not open color dialog";
   }
  }

  void ColoredObject::setColor( QColor color )
  {
   this->color = color;
   emit colorChanged( color );
  }

  void ColoredObject::setInvisible( int checkState )
  {
   switch ( checkState )
   {
    case Qt::Checked :
     this->invisible = true; 
     break;
    case Qt::Unchecked :
     this->invisible = false; 
     break;
    case Qt::PartiallyChecked :
     this->invisible = false; 
     break;
    default :
     this->invisible = false; 
     break;
   }
  }

}
