#include "openclhelper.h"
#include "drainagent.h"
#include "gridview.h"

namespace Langmuir
{

    QImage GridViewGL::drawEnergyLandscape( int layer )
    {
        //Create an empty image of the appropriate size
        QImage img( pPar->gridWidth, pPar->gridHeight, QImage::Format_ARGB32_Premultiplied );
        img.fill( 0 );

        //Create a painter to draw the image
        QPainter painter( &img );
        //Move the origin from top left to bottom right
        painter.scale( 1.0, -1.0 );
        painter.translate( 0.0, -pPar->gridHeight );
        //Resize the painter window to the grid dimensions
        painter.setWindow( QRect( 0, 0, pPar->gridWidth, pPar->gridHeight ) );
        //Set the background to be white
        painter.fillRect ( QRect( 0, 0, pPar->gridWidth, pPar->gridHeight ), Qt::white );
        //Set the trap color to a light gray
        painter.setPen( QColor( 100, 100, 100, 255 ) );
        //Draw the traps as dots
        for ( int i = 0; i < pSim->world()->trapSiteIDs()->size(); i++ )
        {
            //Index of a trap site
            int ndx = pSim->world()->trapSiteIDs()->at( i );
            //If this trap is in the Layer we are drawing...
            if ( pSim->world()->grid()->getLayer( ndx ) == layer )
            {
                //Draw the trap as a point at (x,y) = (col,row)
                painter.drawPoint( QPoint( pSim->world()->grid()->getColumn( ndx ), pSim->world()->grid()->getRow( ndx ) ) );
            }
        }
        //return the new image ( its not copied on return because of how Qt stores images )
        return img;
    }

    GridViewGL::GridViewGL( const QGLFormat &format, QWidget * parent, QString input ): QGLWidget( format, parent )
    {
        //This Widget responds to the keyboard
        setFocusPolicy(Qt::StrongFocus);

        //Create simulation parameters struct
        pPar = new SimulationParameters;

        //See if simulation parameters should be set to default values
        if ( input == "default" )
        {
            //Adjust some of Langmuir's default parameters so things aren't too boring
            pPar->iterationsPrint = 10;
            pPar->gridCharge = true;
            pPar->chargePercentage = 0.02;
            pPar->voltageDrain = 10.0;
            pInput = NULL;
        }
        //Or... get the simulation parameters from an input file
        else
        {
            pInput = new InputParser(input);
            pInput->simulationParameters(pPar);
            //Its probably an error if the input file read contains no parameters
            if ( pInput->getReadCount() == 0 )
            {
                QMessageBox::critical(
                this,
                tr("Langmuir"),
                QString("No parameters were found in the input file.  Langmuir's default parameters will be used."),
                QMessageBox::Ok );

                //Adjust some of Langmuir's default parameters so things aren't too boring
                pPar->iterationsPrint = 10;
                pPar->gridCharge = true;
                pPar->chargePercentage = 0.02;
                pPar->voltageDrain = 10.0;
            }
        }

        //Restrict the value on iterations.print to be low ( it controls how many steps are done everytime updateGL is called )
        if ( pPar->iterationsPrint > 100 )
        {
            QMessageBox::warning(
            this,
            tr("Langmuir"),
            QString("The parameter iterations.print must be <= 100 for GridView to render,"
                    " so it has been changed from %1 to 10.").arg(pPar->iterationsPrint),
            QMessageBox::Ok );
            pPar->iterationsPrint = 10;
        }

        //Create the simulation with the parameters chosen
        pSim = new Simulation(pPar);

        //Set default values
        lastCount = 0;
        translation.setX( 0 );
        translation.setY( 0 );
        translation.setZ( 0 );
        rotation.setX( 0 );
        rotation.setY( 0 );
        rotation.setZ( 0 );
        delta.setX( 5.0 );
        delta.setY( 5.0 );
        delta.setZ( 25.0 );
        thickness = 2.0;
        fov = 60.0;
        pause = true;
        recording = false;
        updateTime = 10;
        step = 0;
        qtimer = new QTimer(this);
        recordTimer = NULL;
        trapsTexture = 0;

        //Connect: make sure updateGL is called every updateTime ms
        connect(qtimer, SIGNAL(timeout()), this, SLOT(timerUpdateGL()));
    }

    GridViewGL::~GridViewGL()
    {
        //Delete the objects who aren't derived from QObject
        delete pPar;
        delete pSim;
        if ( pInput ) { delete pInput; }
    }

    QSize GridViewGL::minimumSizeHint() const
    {
        return QSize(256, 256);
    }

    QSize GridViewGL::sizeHint() const
    {
        return QSize(pPar->gridWidth + 4 * thickness,pPar->gridHeight + 2 * thickness);
    }

    void GridViewGL::initializeGL()
    {
        glEnable(GL_TEXTURE_2D);
        glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
        glClearDepth(1.0f);
        glEnable(GL_DEPTH_TEST);
        glShadeModel(GL_FLAT);

        QImage img;
        img.load("texture.jpeg");
        metalTexture = bindTexture( img );

        trapsTexture = bindTexture( drawEnergyLandscape() );

        base = new Box( this,
                        QVector3D( pPar->gridWidth, pPar->gridHeight, thickness ),
                        QVector3D( 0, 0, -thickness ) );
        base->setTexture( metalTexture );

        source = new Box( this,
                          QVector3D( 2.0*thickness, pPar->gridHeight, thickness + pPar->gridDepth * thickness ),
                          QVector3D( -2.0*thickness, 0, -thickness ) );

        drain = new Box( this,
                         QVector3D( 2.0*thickness, pPar->gridHeight, thickness + pPar->gridDepth * thickness ),
                         QVector3D( pPar->gridWidth, 0, -thickness ) );

        side1 = new Box( this,
                         QVector3D( pPar->gridWidth, thickness, thickness ),
                         QVector3D( 0, -thickness, -thickness ) );

        side2 = new Box( this,
                         QVector3D( pPar->gridWidth, thickness, thickness ),
                         QVector3D( 0, pPar->gridHeight, -thickness ) );

        side3 = new Box( this,
                         QVector3D( 2.0*thickness, thickness, thickness + pPar->gridDepth * thickness ),
                         QVector3D( -2.0*thickness, -thickness, -thickness ) );

        side4 = new Box( this,
                         QVector3D( 2.0*thickness, thickness, thickness + pPar->gridDepth * thickness ),
                         QVector3D( -2.0*thickness, pPar->gridHeight, -thickness ) );

        side5 = new Box( this,
                         QVector3D( 2.0*thickness, thickness, thickness + pPar->gridDepth * thickness ),
                         QVector3D( pPar->gridWidth, -thickness, -thickness ) );

        side6 = new Box( this,
                         QVector3D( 2.0*thickness, thickness, thickness + pPar->gridDepth * thickness ),
                         QVector3D( pPar->gridWidth, pPar->gridHeight, -thickness ) );

        background = new ColoredObject( this );
        ambientLight = new ColoredObject( this );
        diffuseLight = new ColoredObject( this );
        specularLight = new ColoredObject( this );

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
            int site = charges->at(i)->site();
            pointBuffer[ i * 4 + 0 ] = grid->getColumn(site) + 0.5;
            pointBuffer[ i * 4 + 1 ] = grid->getRow(site) + 0.5;
            pointBuffer[ i * 4 + 2 ] = (grid->getLayer(site) + 0.5) * thickness;
        }
        carriers = new PointArray(this,pointBuffer);

        QList< int > *defectList = pSim->world()->defectSiteIDs();
        QVector< float > xyz( defectList->size() * 4, 1 );
        for ( int i = 0; i < defectList->size(); i++ )
        {
            xyz[ i * 4 + 0 ] = grid->getColumn(defectList->at(i)) + 0.5;
            xyz[ i * 4 + 1 ] = grid->getRow(defectList->at(i)) + 0.5;
            xyz[ i * 4 + 2 ] = (grid->getLayer(defectList->at(i)) + 0.5) * thickness;
            xyz[ i * 4 + 3 ] = 1;
        }
        defects  = new PointArray(this,xyz);

        recordDialog = new RecordDialog(this);

        translation.setX( 0 );
        translation.setY( 0 );
        translation.setZ( -(pPar->gridHeight)-thickness );
        rotation.setX( 0 );
        rotation.setY( 0 );
        rotation.setZ( 0 );

        emit initialized();
        emit xTranslationChanged( translation.x() );
        emit yTranslationChanged( translation.y() );
        emit zTranslationChanged( translation.z() );
        emit xRotationChanged( rotation.x() );
        emit yRotationChanged( rotation.y() );
        emit zRotationChanged( rotation.z() );

        base->setColor(QColor(107,221,89,255));
        source->setColor(QColor(0,0,255,255));
        drain->setColor(QColor(255,0,0,255));
        side1->setColor(QColor(255,255,255,255));
        side2->setColor(QColor(255,255,255,255));
        side3->setColor(QColor(255,255,255,255));
        side4->setColor(QColor(255,255,255,255));
        side5->setColor(QColor(255,255,255,255));
        side6->setColor(QColor(255,255,255,255));
        background->setColor(QColor(28,164,255,255));
        carriers->setColor(QColor(255,0,0,255));
        defects->setColor(QColor(0,0,0,255));
        ambientLight->setColor(QColor(85,85,85,255));
        diffuseLight->setColor(QColor(170,170,170,255));
        specularLight->setColor(QColor(255,255,255,255));

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0,GL_AMBIENT,ambientLight->getLight());
        glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuseLight->getLight());
        glLightfv(GL_LIGHT0,GL_SPECULAR,specularLight->getLight());

        carriers->setPointSize(0.5f);
        defects->setPointSize(0.5f);

        if ( pPar->interactionCoulomb )
        {
            emit coulombStatusChanged( Qt::Checked );
        }
        else
        {
            emit coulombStatusChanged( Qt::Unchecked );
        }

        if ( pPar->useOpenCL )
        {
            emit openCLStatusChanged( Qt::Checked );
        }
        else
        {
            emit openCLStatusChanged( Qt::Unchecked );
        }

        setIterationsPrint( pPar->iterationsPrint );
        setTimerInterval( updateTime );

        qtimer->start(updateTime);
    }

    void GridViewGL::timerUpdateGL()
    {
        if ( ! pause )
        {
            pSim->performIterations( pPar->iterationsPrint );
            //double current = ( pSim->totalChargesAccepted() - lastCount ) / ( double( pPar->iterationsPrint ) );
            lastCount = pSim->world()->drain()->acceptedCharges();
            step += pPar->iterationsPrint;

            QList< ChargeAgent* > *charges = pSim->world()->charges();
            Grid *grid = pSim->world()->grid();

            for ( int i = 0; i < charges->size(); i++ )
            {
                int site = charges->at(i)->site();
                pointBuffer[ i * 4 + 0 ] = grid->getColumn(site) + 0.5;
                pointBuffer[ i * 4 + 1 ] = grid->getRow(site) + 0.5;
                pointBuffer[ i * 4 + 2 ] = (grid->getLayer(site) + 0.5) * thickness;
            }

            carriers->update( pointBuffer, charges->size() * 4 );

            emit stepChanged( step );
        }
        updateGL();

        if ( recording )
        {
            timerRecordShot();
        }
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

    void GridViewGL::togglePauseStatus()
    {
        pause = !( pause );
        if ( pause )
        {
            emit pauseChanged( QString("play") );
            //emit statusMessage( "paused" );
        }
        else
        {
            emit pauseChanged( QString("pause") );
            //emit statusMessage( "playing" );
        }
    }

    void GridViewGL::resetView()
    {
        translation.setX( 0.0 );
        translation.setY( 0.0 );
        translation.setZ( -(pPar->gridHeight)-thickness );
        rotation.setX( 0 );
        rotation.setY( 0 );
        rotation.setZ( 0 );
        emit xTranslationChanged( translation.x() );
        emit yTranslationChanged( translation.y() );
        emit zTranslationChanged( translation.z() );
        emit xRotationChanged( rotation.x() );
        emit yRotationChanged( rotation.y() );
        emit zRotationChanged( rotation.z() );
    }

    void GridViewGL::setTimerInterval( int time )
    {
        updateTime = time;
        delete qtimer;
        qtimer = new QTimer( this );
        connect(qtimer, SIGNAL(timeout()), this, SLOT(timerUpdateGL()));
        qtimer->start( updateTime );
        emit timerIntervalChanged( updateTime );
    }

    void GridViewGL::setIterationsPrint( int iterationsPrint )
    {
        if ( iterationsPrint > 0 && iterationsPrint <= 100 )
        {
            pPar->iterationsPrint = iterationsPrint;
            emit iterationsPrintChanged( iterationsPrint );
        }
    }

    void GridViewGL::toggleCoulombStatus(int checkState)
    {
        switch ( checkState )
        {
            case Qt::Checked :
            {
                pPar->interactionCoulomb = true;
                emit coulombStatusChanged( Qt::Checked );
                break;
            }
            default :
            {
                pPar->interactionCoulomb = false;
                pSim->world()->opencl()->toggleOpenCL(false);
                emit openCLStatusChanged( Qt::Unchecked );
                emit coulombStatusChanged( Qt::Unchecked );
                break;
            }
        }
    }

    void GridViewGL::toggleTrapsTexture(int checkState)
    {

        switch ( checkState )
        {
            case Qt::Checked :
            {
                base->setTexture( metalTexture );
                break;
            }
            default :
            {
                base->setTexture( trapsTexture );
                break;
            }
        }

    }

    void GridViewGL::toggleOpenCLStatus(int checkState)
    {
        switch ( checkState )
        {
            case Qt::Checked :
            {
                if ( ! ( pSim->world()->opencl()->toggleOpenCL(true) ) )
                {
                    bool resume = false;
                    if ( ! pause )
                    {
                        togglePauseStatus();
                        resume = true;
                    }
                    if ( ! pPar->okCL )
                    {
                        QMessageBox::warning( this, tr( "Langmuir" ), QString("Can not turn OpenCL.  This platform does not allow openCL."));
                    }
                    else
                    {
                        if ( ! pPar->interactionCoulomb )
                        {

                            QMessageBox::warning( this, tr( "Langmuir" ), QString("Can not turn OpenCL.  Coulomb interations are turned off."));
                        }
                    }
                    if ( resume )
                    {
                        togglePauseStatus();
                    }
                    emit openCLStatusChanged( Qt::Unchecked );
                }
                else
                {
                    emit openCLStatusChanged( Qt::Checked );
                }
                break;
            }
            default :
            {
                pSim->world()->opencl()->toggleOpenCL(false);
                emit openCLStatusChanged( Qt::Unchecked );
                break;
            }
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
                togglePauseStatus();
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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
        glLightfv(GL_LIGHT0,GL_AMBIENT,ambientLight->getLight());
        glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuseLight->getLight());
        glLightfv(GL_LIGHT0,GL_SPECULAR,specularLight->getLight());
        glClearDepth(1.0);
        glClearColor(background->getColor().redF(),background->getColor().greenF(),background->getColor().blueF(),background->getColor().alphaF());
        glLoadIdentity();
        glTranslatef(translation.x(),translation.y(),translation.z());
        glRotatef(rotation.x(),1.0,0.0,0.0);
        glRotatef(rotation.y(),0.0,1.0,0.0);
        glRotatef(rotation.z(),0.0,0.0,1.0);
        glPushMatrix();
        glTranslatef( -0.5 * ( pPar->gridWidth + 4.0 * thickness ), - 0.5 * ( pPar->gridHeight + 2.0 * thickness ), 0 );
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
        defects->draw( pSim->world()->defectSiteIDs()->size(), this->height(), fov );
        glPopMatrix();
    }

    Box::Box( QObject *parent, QVector3D dimensions, QVector3D origin ) : ColoredObject(parent), tid( 0 )
    {
        QVector<float> varray(72);
        QVector<float> narray(72);
        QVector<float> tarray(48);

        varray[ 0] = 1.0f; narray[ 0] =  0.0f; tarray[ 0] =  1.0f;
        varray[ 1] = 1.0f; narray[ 1] =  1.0f; tarray[ 1] =  0.0f;
        varray[ 2] = 0.0f; narray[ 2] =  0.0f; 

        varray[ 3] = 0.0f; narray[ 3] =  0.0f; tarray[ 2] =  0.0f;
        varray[ 4] = 1.0f; narray[ 4] =  1.0f; tarray[ 3] =  0.0f;
        varray[ 5] = 0.0f; narray[ 5] =  0.0f; 

        varray[ 6] = 0.0f; narray[ 6] =  0.0f; tarray[ 4] =  0.0f;
        varray[ 7] = 1.0f; narray[ 7] =  1.0f; tarray[ 5] =  1.0f;
        varray[ 8] = 1.0f; narray[ 8] =  0.0f; 

        varray[ 9] = 1.0f; narray[ 9] =  0.0f; tarray[ 6] =  1.0f;
        varray[10] = 1.0f; narray[10] =  1.0f; tarray[ 7] =  1.0f;
        varray[11] = 1.0f; narray[11] =  0.0f; 
        /*****************************************************************/
        varray[12] = 1.0f; narray[12] =  0.0f; tarray[ 8] =  1.0f;
        varray[13] = 0.0f; narray[13] = -1.0f; tarray[ 9] =  1.0f;
        varray[14] = 1.0f; narray[14] =  0.0f; 

        varray[15] = 0.0f; narray[15] =  0.0f; tarray[10] =  0.0f;
        varray[16] = 0.0f; narray[16] = -1.0f; tarray[11] =  1.0f;
        varray[17] = 1.0f; narray[17] =  0.0f; 

        varray[18] = 0.0f; narray[18] =  0.0f; tarray[12] =  0.0f;
        varray[19] = 0.0f; narray[19] = -1.0f; tarray[13] =  0.0f;
        varray[20] = 0.0f; narray[20] =  0.0f; 

        varray[21] = 1.0f; narray[21] =  0.0f; tarray[14] =  1.0f;
        varray[22] = 0.0f; narray[22] = -1.0f; tarray[15] =  0.0f;
        varray[23] = 0.0f; narray[23] =  0.0f; 
        /*****************************************************************/
        varray[24] = 1.0f; narray[24] =  0.0f; tarray[16] =  1.0f;
        varray[25] = 1.0f; narray[25] =  0.0f; tarray[17] =  1.0f;
        varray[26] = 1.0f; narray[26] =  1.0f; 

        varray[27] = 0.0f; narray[27] =  0.0f; tarray[18] =  0.0f;
        varray[28] = 1.0f; narray[28] =  0.0f; tarray[19] =  1.0f;
        varray[29] = 1.0f; narray[29] =  1.0f; 

        varray[30] = 0.0f; narray[30] =  0.0f; tarray[20] =  0.0f;
        varray[31] = 0.0f; narray[31] =  0.0f; tarray[21] =  0.0f;
        varray[32] = 1.0f; narray[32] =  1.0f; 

        varray[33] = 1.0f; narray[33] =  0.0f; tarray[22] =  1.0f;
        varray[34] = 0.0f; narray[34] =  0.0f; tarray[23] =  0.0f;
        varray[35] = 1.0f; narray[35] =  1.0f; 
        /*****************************************************************/
        varray[36] = 1.0f; narray[36] =  0.0f; tarray[24] =  1.0f;
        varray[37] = 0.0f; narray[37] =  0.0f; tarray[25] =  0.0f;
        varray[38] = 0.0f; narray[38] = -1.0f; 

        varray[39] = 0.0f; narray[39] =  0.0f; tarray[26] =  0.0f;
        varray[40] = 0.0f; narray[40] =  0.0f; tarray[27] =  0.0f;
        varray[41] = 0.0f; narray[41] = -1.0f; 

        varray[42] = 0.0f; narray[42] =  0.0f; tarray[28] =  0.0f;
        varray[43] = 1.0f; narray[43] =  0.0f; tarray[29] =  1.0f;
        varray[44] = 0.0f; narray[44] = -1.0f; 

        varray[45] = 1.0f; narray[45] =  0.0f; tarray[30] =  1.0f;
        varray[46] = 1.0f; narray[46] =  0.0f; tarray[31] =  1.0f;
        varray[47] = 0.0f; narray[47] = -1.0f; 
        /*****************************************************************/
        varray[48] = 0.0f; narray[48] = -1.0f; tarray[32] =  1.0f;
        varray[49] = 1.0f; narray[49] =  0.0f; tarray[33] =  1.0f;
        varray[50] = 1.0f; narray[50] =  0.0f; 

        varray[51] = 0.0f; narray[51] = -1.0f; tarray[34] =  1.0f;
        varray[52] = 1.0f; narray[52] =  0.0f; tarray[35] =  0.0f;
        varray[53] = 0.0f; narray[53] =  0.0f; 

        varray[54] = 0.0f; narray[54] = -1.0f; tarray[36] =  0.0f;
        varray[55] = 0.0f; narray[55] =  0.0f; tarray[37] =  0.0f;
        varray[56] = 0.0f; narray[56] =  0.0f; 

        varray[57] = 0.0f; narray[57] = -1.0f; tarray[38] =  0.0f;
        varray[58] = 0.0f; narray[58] =  0.0f; tarray[39] =  1.0f;
        varray[59] = 1.0f; narray[59] =  0.0f; 
        /*****************************************************************/
        varray[60] = 1.0f; narray[60] =  1.0f; tarray[40] =  1.0f;
        varray[61] = 1.0f; narray[61] =  0.0f; tarray[41] =  0.0f;
        varray[62] = 0.0f; narray[62] =  0.0f; 

        varray[63] = 1.0f; narray[63] =  1.0f; tarray[42] =  1.0f;
        varray[64] = 1.0f; narray[64] =  0.0f; tarray[43] =  1.0f;
        varray[65] = 1.0f; narray[65] =  0.0f; 

        varray[66] = 1.0f; narray[66] =  1.0f; tarray[44] =  0.0f;
        varray[67] = 0.0f; narray[67] =  0.0f; tarray[45] =  1.0f;
        varray[68] = 1.0f; narray[68] =  0.0f; 

        varray[69] = 1.0f; narray[69] =  1.0f; tarray[46] =  0.0f;
        varray[70] = 0.0f; narray[70] =  0.0f; tarray[47] =  0.0f;
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

        vBuffer.setUsagePattern( QGLBuffer::StaticDraw );
        vBuffer.create();
        vBuffer.bind();
        vBuffer.allocate( &varray[0], varray.size() * sizeof( float ) );
        vBuffer.release();

        nBuffer.setUsagePattern( QGLBuffer::StaticDraw );
        nBuffer.create();
        nBuffer.bind();
        nBuffer.allocate( &narray[0], narray.size() * sizeof( float ) );
        nBuffer.release();

        tBuffer.setUsagePattern( QGLBuffer::StaticDraw );
        tBuffer.create();
        tBuffer.bind();
        tBuffer.allocate( &tarray[0], tarray.size() * sizeof( float ) );
        tBuffer.release();

        setColor( QColor(0,0,0,255) );

    }

    Box::~Box()
    {
        vBuffer.destroy();
        nBuffer.destroy();
        tBuffer.destroy();
    }

    void Box::draw()
    {
        if ( ! invisible )
        {
            glEnableClientState(GL_VERTEX_ARRAY);
            vBuffer.bind();
            glVertexPointer(3,GL_FLOAT,0,0);

            glEnableClientState(GL_NORMAL_ARRAY);
            nBuffer.bind();
            glNormalPointer(GL_FLOAT,0,0);

            if( tid )
            {
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                tBuffer.bind();
                glTexCoordPointer(2,GL_FLOAT,0,0);
                glBindTexture(GL_TEXTURE_2D,tid);
            }

            glMaterialfv(GL_FRONT, GL_AMBIENT, this->getLight() );
            glMaterialfv(GL_FRONT, GL_DIFFUSE, this->getLight() );
            glMaterialf(GL_FRONT, GL_SHININESS, 128);

            glDrawArrays(GL_QUADS,0,24);

            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_NORMAL_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);

            vBuffer.release();
            nBuffer.release();
            tBuffer.release();

            glBindTexture(GL_TEXTURE_2D,0);
        }
    }

    void Box::setTexture( int tid )
    {
        this->tid = tid;
    }

    PointArray::PointArray( QObject *parent, QVector<float>& xyz) : ColoredObject(parent)
    {
        const char *vsource =
            " uniform float pointRadius;\n"
            " uniform float pointScale;\n"
            " uniform float densityScale;\n"
            " uniform float densityOffset;\n"
            " varying vec3 posEye;\n"
            " void main()\n"
            " {\n"
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
            " uniform float pointRadius;\n"
            " varying vec3 posEye;\n"
            " void main()\n"
            " {\n"
            "     const vec3 lightDir = vec3(0.577, 0.577, 0.577);\n"
            "     const float shininess = 40.0;\n"
            "\n"
            "     vec3 n;\n"
            "     n.xy = gl_TexCoord[0].xy*vec2(2.0, -2.0) + vec2(-1.0, 1.0);\n"
            "     float mag = dot(n.xy, n.xy);\n"
            "     if (mag > 1.0) discard;\n"
            "     n.z = sqrt(1.0-mag);\n"
            "\n"
            "     vec3 spherePosEye = posEye + n*pointRadius;\n"
            "\n"
            "     float diffuse = max(0.0, dot(lightDir, n));\n"
            "     vec3 v = normalize(-spherePosEye);\n"
            "     vec3 h = normalize(lightDir + v);\n"
            "     float specular = pow(max(0.0, dot(n, h)), shininess);\n"
            "\n"
            "     gl_FragColor = gl_Color * diffuse + specular;\n"
            " }\n";

        if ( !program.addShaderFromSourceCode( QGLShader::Vertex, vsource ) )
        {
            qWarning("could not add vertex shader");
        }
        if ( !program.addShaderFromSourceCode( QGLShader::Fragment, fsource ) )
        {
            qWarning("could not add fragment shader");
        }
        if ( !program.link() )
        {
            qWarning("could not link shaders");
        }

        vBuffer.setUsagePattern( QGLBuffer::DynamicDraw );
        vBuffer.create();
        vBuffer.bind();
        if ( xyz.size() > 0 )
        {
            vBuffer.allocate( &xyz[0], xyz.size() * sizeof( float ) );
        }
        vBuffer.release();

        setColor( QColor(0,0,0,255) );
        setPointSize( 0.0f );
        setSpheres( Qt::Unchecked );
    }

    PointArray::~PointArray()
    {
        vBuffer.destroy();
    }

    void PointArray::update( QVector<float> &xyz, int size )
    {
        vBuffer.bind();
        vBuffer.write( 0, &xyz[0], size * sizeof( float ) );
        vBuffer.release();
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

    void PointArray::draw( int size, int height, float fov )
    {
        if ( ! invisible && size > 0 )
        {
            float black[4] = { 0, 0, 0, 1 };
            if ( spheres )
            {
                glEnable(GL_POINT_SPRITE);
                glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
                glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
                glDepthMask(GL_TRUE);
                glEnable(GL_DEPTH_TEST);
                program.bind();
                program.setUniformValue( "pointScale", height / tanf( fov * 0.5 * M_PI / 180.0 ) );
                program.setUniformValue( "pointRadius", pointSize );
                glColor4f(color.redF(),color.greenF(),color.blueF(),color.alphaF());
                glMaterialfv(GL_FRONT, GL_AMBIENT, this->getLight() );
                glMaterialfv(GL_FRONT, GL_DIFFUSE, this->getLight() );
                glMaterialfv(GL_FRONT, GL_SPECULAR, black );
                glMaterialf(GL_FRONT, GL_SHININESS, 96);
                vBuffer.bind();
                glVertexPointer(4, GL_FLOAT, 0, 0);
                glEnableClientState(GL_VERTEX_ARRAY);
                glDrawArrays(GL_POINTS, 0, size);
                vBuffer.release();
                glDisableClientState(GL_VERTEX_ARRAY);
                glDisableClientState(GL_COLOR_ARRAY);
                program.release();
                glDisable(GL_POINT_SPRITE);
            }
            else
            {
                glPointSize(pointSize);
                glEnable(GL_POINT_SMOOTH);
                glMaterialfv(GL_FRONT, GL_AMBIENT, this->getLight() );
                glMaterialfv(GL_FRONT, GL_DIFFUSE, this->getLight() );
                glMaterialfv(GL_FRONT, GL_SPECULAR, black );
                glMaterialf(GL_FRONT, GL_SHININESS, 96);
                glEnableClientState(GL_VERTEX_ARRAY);
                vBuffer.bind();
                glVertexPointer(4,GL_FLOAT,0,0);
                glDrawArrays(GL_POINTS,0,size);
                glDisableClientState(GL_VERTEX_ARRAY);
                glDisable(GL_POINT_SMOOTH);
                vBuffer.release();
            }
        }
    }

    MainWindow::MainWindow( QString input ) : QMainWindow( )
    {
        setWindowTitle(tr("Langmuir View"));

        QGLFormat format;
        format.setVersion(2,0);
        glWidget = new GridViewGL(format,this,input);
        setCentralWidget( glWidget );
        connect( glWidget, SIGNAL( initialized() ), this, SLOT( setConnections() ) );

        QDockWidget *ndock = new QDockWidget("navigator",this);
        ndock->setStyleSheet( QString("QDockWidget{ color: white; border: 1px solid black; } QDockWidget::title{ background: black; padding-left: 5px; }") );
        ndock->setFeatures( QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable );
        ndock->setAllowedAreas(Qt::RightDockWidgetArea);
        navigator = new Navigator(ndock);
        ndock->setWidget( navigator );
        addDockWidget( Qt::RightDockWidgetArea, ndock );

        QDockWidget *cdock = new QDockWidget("controls",this);
        cdock->setStyleSheet( QString("QDockWidget{ color: white; border: 1px solid black; } QDockWidget::title{ background: black; padding-left: 5px; }") );
        cdock->setFeatures( QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable );
        cdock->setAllowedAreas(Qt::RightDockWidgetArea);
        controls = new Controls(cdock);
        cdock->setWidget(controls);
        addDockWidget(Qt::RightDockWidgetArea,cdock);

        QDockWidget *sdock = new QDockWidget("options",this);
        sdock->setStyleSheet( QString("QDockWidget{ color: white; border: 1px solid black; } QDockWidget::title{ background: black; padding-left: 5px; }") );
        sdock->setFeatures( QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable );
        sdock->setAllowedAreas(Qt::RightDockWidgetArea);
        sceneOptions = new SceneOptions(this);
        sdock->setWidget( sceneOptions );
        addDockWidget( Qt::RightDockWidgetArea, sdock );

        statusBar()->showMessage( "press play button to start simulation" );

        int maxWidth = qMax( sceneOptions->width(), qMax( navigator->width(), controls->width() ) );

        navigator->resize( maxWidth, navigator->height() );
        controls->resize( maxWidth, controls->height() );
        sceneOptions->resize( maxWidth, sceneOptions->height() );

        navigator->setMaximumSize( maxWidth, navigator->height() );
        controls->setMaximumSize( maxWidth, controls->height() );
        sceneOptions->setMaximumSize( maxWidth, sceneOptions->height() );

        navigator->setMinimumSize( maxWidth, navigator->height() );
        controls->setMinimumSize( maxWidth, controls->height() );
        sceneOptions->setMinimumSize( maxWidth, sceneOptions->height() );

        resize( width() + maxWidth, height() );
    }

    void MainWindow::setConnections()
    {
        connect( navigator->spinBoxes[0], SIGNAL( valueChanged(double) ), glWidget, SLOT( setXTranslation(double) ) );
        connect( navigator->spinBoxes[1], SIGNAL( valueChanged(double) ), glWidget, SLOT( setYTranslation(double) ) );
        connect( navigator->spinBoxes[2], SIGNAL( valueChanged(double) ), glWidget, SLOT( setZTranslation(double) ) );
        connect( navigator->spinBoxes[3], SIGNAL( valueChanged(double) ), glWidget, SLOT( setXRotation(double) ) );
        connect( navigator->spinBoxes[4], SIGNAL( valueChanged(double) ), glWidget, SLOT( setYRotation(double) ) );
        connect( navigator->spinBoxes[5], SIGNAL( valueChanged(double) ), glWidget, SLOT( setZRotation(double) ) );
        connect( navigator->buttons[0], SIGNAL( clicked() ), glWidget, SLOT( resetView() ) );
        connect( glWidget, SIGNAL( xTranslationChanged(double) ), navigator->spinBoxes[0], SLOT( setValueSlot(double) ) );
        connect( glWidget, SIGNAL( yTranslationChanged(double) ), navigator->spinBoxes[1], SLOT( setValueSlot(double) ) );
        connect( glWidget, SIGNAL( zTranslationChanged(double) ), navigator->spinBoxes[2], SLOT( setValueSlot(double) ) );
        connect( glWidget, SIGNAL( xRotationChanged(double) ), navigator->spinBoxes[3], SLOT( setValueSlot(double) ) );
        connect( glWidget, SIGNAL( yRotationChanged(double) ), navigator->spinBoxes[4], SLOT( setValueSlot(double) ) );
        connect( glWidget, SIGNAL( zRotationChanged(double) ), navigator->spinBoxes[5], SLOT( setValueSlot(double) ) );
        connect( controls->buttons[0], SIGNAL( clicked() ), glWidget, SLOT( togglePauseStatus() ) );
        connect( glWidget, SIGNAL( pauseChanged( QString ) ), controls->buttons[0], SLOT( setTextSlot( QString ) ) );
        connect( controls->buttons[1], SIGNAL( clicked() ), glWidget, SLOT( toggleRecording() ) );
        connect( glWidget, SIGNAL( recordChanged( QString ) ), controls->buttons[1], SLOT( setTextSlot( QString ) ) );
        connect( glWidget, SIGNAL( recordChangedColor( QColor ) ), controls->buttons[1], SLOT( setColorSlot( QColor ) ) );
        connect( controls->buttons[2], SIGNAL( clicked() ), glWidget, SLOT( screenShot() ) );
        connect( controls->spinBoxes[0], SIGNAL( valueChanged( int ) ), glWidget, SLOT( setTimerInterval( int ) ) );
        connect( glWidget, SIGNAL( timerIntervalChanged( int ) ), controls->spinBoxes[0], SLOT( setValueSlot( int ) ) );
        connect( controls->spinBoxes[1], SIGNAL( valueChanged( int ) ), glWidget, SLOT( setIterationsPrint( int ) ) );
        connect( glWidget, SIGNAL( iterationsPrintChanged( int ) ), controls->spinBoxes[1], SLOT( setValueSlot( int ) ) );
        connect( controls->checkBoxes[0], SIGNAL( stateChanged( int ) ), glWidget, SLOT( toggleOpenCLStatus( int ) ) );
        connect( glWidget, SIGNAL( openCLStatusChanged( int ) ), controls->checkBoxes[0], SLOT( setValueSlot( int ) ) );
        connect( controls->checkBoxes[1], SIGNAL( stateChanged( int ) ), glWidget, SLOT( toggleCoulombStatus( int ) ) );
        connect( glWidget, SIGNAL( coulombStatusChanged( int ) ), controls->checkBoxes[1], SLOT( setValueSlot( int ) ) );
        connect( glWidget, SIGNAL( stepChanged( int ) ), controls->lcdNumbers[0], SLOT( display( int ) ) );
        connect( glWidget, SIGNAL( statusMessage( QString, int ) ), statusBar(), SLOT( showMessage( QString, int ) ) );
        glWidget->carriers->setColorDialog( sceneOptions->colorDialog );
        glWidget->defects->setColorDialog( sceneOptions->colorDialog );
        glWidget->source->setColorDialog( sceneOptions->colorDialog );
        glWidget->drain->setColorDialog( sceneOptions->colorDialog );
        glWidget->base->setColorDialog( sceneOptions->colorDialog );
        glWidget->background->setColorDialog( sceneOptions->colorDialog );
        glWidget->ambientLight->setColorDialog( sceneOptions->colorDialog );
        glWidget->diffuseLight->setColorDialog( sceneOptions->colorDialog );
        glWidget->specularLight->setColorDialog( sceneOptions->colorDialog );
        glWidget->side1->setColorDialog( sceneOptions->colorDialog );
        glWidget->side2->setColorDialog( sceneOptions->colorDialog );
        glWidget->side3->setColorDialog( sceneOptions->colorDialog );
        glWidget->side4->setColorDialog( sceneOptions->colorDialog );
        glWidget->side5->setColorDialog( sceneOptions->colorDialog );
        glWidget->side6->setColorDialog( sceneOptions->colorDialog );
        connect( sceneOptions->buttons[0], SIGNAL( clicked() ), glWidget->carriers, SLOT( openColorDialog() ) );
        connect( sceneOptions->buttons[1], SIGNAL( clicked() ), glWidget->defects, SLOT( openColorDialog() ) );
        connect( sceneOptions->buttons[2], SIGNAL( clicked() ), glWidget->source, SLOT( openColorDialog() ) );
        connect( sceneOptions->buttons[3], SIGNAL( clicked() ), glWidget->drain, SLOT( openColorDialog() ) );
        connect( sceneOptions->buttons[4], SIGNAL( clicked() ), glWidget->base, SLOT( openColorDialog() ) );
        connect( sceneOptions->buttons[5], SIGNAL( clicked() ), glWidget->background, SLOT( openColorDialog() ) );
        connect( sceneOptions->buttons[6], SIGNAL( clicked() ), glWidget->side1, SLOT( openColorDialog() ) );
        connect( sceneOptions->buttons[7], SIGNAL( clicked() ), glWidget->ambientLight, SLOT( openColorDialog() ) );
        connect( sceneOptions->buttons[8], SIGNAL( clicked() ), glWidget->diffuseLight, SLOT( openColorDialog() ) );
        connect( sceneOptions->buttons[9], SIGNAL( clicked() ), glWidget->specularLight, SLOT( openColorDialog() ) );
        connect( glWidget->side1, SIGNAL( colorChanged( QColor ) ), glWidget->side2, SLOT( setColor( QColor ) ) );
        connect( glWidget->side1, SIGNAL( colorChanged( QColor ) ), glWidget->side3, SLOT( setColor( QColor ) ) );
        connect( glWidget->side1, SIGNAL( colorChanged( QColor ) ), glWidget->side4, SLOT( setColor( QColor ) ) );
        connect( glWidget->side1, SIGNAL( colorChanged( QColor ) ), glWidget->side5, SLOT( setColor( QColor ) ) );
        connect( glWidget->side1, SIGNAL( colorChanged( QColor ) ), glWidget->side6, SLOT( setColor( QColor ) ) );
        connect( glWidget->carriers, SIGNAL( colorChanged( QColor ) ), sceneOptions->buttons[0], SLOT( setColorSlot( QColor ) ) );
        connect( glWidget->defects, SIGNAL( colorChanged( QColor ) ), sceneOptions->buttons[1], SLOT( setColorSlot( QColor ) ) );
        connect( glWidget->source, SIGNAL( colorChanged( QColor ) ), sceneOptions->buttons[2], SLOT( setColorSlot( QColor ) ) );
        connect( glWidget->drain, SIGNAL( colorChanged( QColor ) ), sceneOptions->buttons[3], SLOT( setColorSlot( QColor ) ) );
        connect( glWidget->base, SIGNAL( colorChanged( QColor ) ), sceneOptions->buttons[4], SLOT( setColorSlot( QColor ) ) );
        connect( glWidget->background, SIGNAL( colorChanged( QColor ) ), sceneOptions->buttons[5], SLOT( setColorSlot( QColor ) ) );
        connect( glWidget->side1, SIGNAL( colorChanged( QColor ) ), sceneOptions->buttons[6], SLOT( setColorSlot( QColor ) ) );
        connect( glWidget->side2, SIGNAL( colorChanged( QColor ) ), sceneOptions->buttons[6], SLOT( setColorSlot( QColor ) ) );
        connect( glWidget->side3, SIGNAL( colorChanged( QColor ) ), sceneOptions->buttons[6], SLOT( setColorSlot( QColor ) ) );
        connect( glWidget->side4, SIGNAL( colorChanged( QColor ) ), sceneOptions->buttons[6], SLOT( setColorSlot( QColor ) ) );
        connect( glWidget->side5, SIGNAL( colorChanged( QColor ) ), sceneOptions->buttons[6], SLOT( setColorSlot( QColor ) ) );
        connect( glWidget->side6, SIGNAL( colorChanged( QColor ) ), sceneOptions->buttons[6], SLOT( setColorSlot( QColor ) ) );
        connect( glWidget->ambientLight, SIGNAL( colorChanged( QColor ) ), sceneOptions->buttons[7], SLOT( setColorSlot( QColor ) ) );
        connect( glWidget->diffuseLight, SIGNAL( colorChanged( QColor ) ), sceneOptions->buttons[8], SLOT( setColorSlot( QColor ) ) );
        connect( glWidget->specularLight, SIGNAL( colorChanged( QColor ) ), sceneOptions->buttons[9], SLOT( setColorSlot( QColor ) ) );
        connect( sceneOptions->checkBoxes[0], SIGNAL( stateChanged(int) ), glWidget->carriers, SLOT( setInvisible(int) ) );
        connect( sceneOptions->checkBoxes[1], SIGNAL( stateChanged(int) ), glWidget->defects, SLOT( setInvisible(int) ) );
        connect( sceneOptions->checkBoxes[2], SIGNAL( stateChanged(int) ), glWidget->source, SLOT( setInvisible(int) ) );
        connect( sceneOptions->checkBoxes[3], SIGNAL( stateChanged(int) ), glWidget->drain, SLOT( setInvisible(int) ) );
        connect( sceneOptions->checkBoxes[4], SIGNAL( stateChanged(int) ), glWidget->base, SLOT( setInvisible(int) ) );
        connect( sceneOptions->checkBoxes[5], SIGNAL( stateChanged(int) ), glWidget->side1, SLOT( setInvisible(int) ) );
        connect( sceneOptions->checkBoxes[5], SIGNAL( stateChanged(int) ), glWidget->side2, SLOT( setInvisible(int) ) );
        connect( sceneOptions->checkBoxes[5], SIGNAL( stateChanged(int) ), glWidget->side3, SLOT( setInvisible(int) ) );
        connect( sceneOptions->checkBoxes[5], SIGNAL( stateChanged(int) ), glWidget->side4, SLOT( setInvisible(int) ) );
        connect( sceneOptions->checkBoxes[5], SIGNAL( stateChanged(int) ), glWidget->side5, SLOT( setInvisible(int) ) );
        connect( sceneOptions->checkBoxes[5], SIGNAL( stateChanged(int) ), glWidget->side6, SLOT( setInvisible(int) ) );
        connect( sceneOptions->checkBoxes[6], SIGNAL( stateChanged(int) ), glWidget->carriers, SLOT( setSpheres(int) ) );
        connect( sceneOptions->checkBoxes[7], SIGNAL( stateChanged(int) ), glWidget->defects, SLOT( setSpheres(int) ) );
        connect( sceneOptions->checkBoxes[8], SIGNAL( stateChanged(int) ), glWidget->ambientLight, SLOT( setInvisible(int) ) );
        connect( sceneOptions->checkBoxes[9], SIGNAL( stateChanged(int) ), glWidget->diffuseLight, SLOT( setInvisible(int) ) );
        connect( sceneOptions->checkBoxes[10], SIGNAL( stateChanged(int) ), glWidget->specularLight, SLOT( setInvisible(int) ) );
        connect( sceneOptions->checkBoxes[11], SIGNAL( stateChanged(int) ), glWidget, SLOT( toggleTrapsTexture(int) ) );
        connect( sceneOptions->spinBoxes[0], SIGNAL( valueChanged(double) ), glWidget->carriers, SLOT( setPointSize(double) ) );
        connect( sceneOptions->spinBoxes[1], SIGNAL( valueChanged(double) ), glWidget->defects, SLOT( setPointSize(double) ) );
    }

    Navigator::Navigator( QWidget *parent ) : QWidget( parent )
    {
        setWindowTitle("navigator");

        labels.push_back( new QLabel( this ) );
        labels.push_back( new QLabel( this ) );
        labels.push_back( new QLabel( this ) );
        labels.push_back( new QLabel( this ) );
        labels.push_back( new QLabel( this ) );
        labels.push_back( new QLabel( this ) );
        labels[0]->setText("x");
        labels[1]->setText("y");
        labels[2]->setText("z");
        labels[3]->setText("a");
        labels[4]->setText("b");
        labels[5]->setText("c");

        spinBoxes.push_back( new DSpinBox( this ) );
        spinBoxes.push_back( new DSpinBox( this ) );
        spinBoxes.push_back( new DSpinBox( this ) );
        spinBoxes.push_back( new DSpinBox( this ) );
        spinBoxes.push_back( new DSpinBox( this ) );
        spinBoxes.push_back( new DSpinBox( this ) );

        spinBoxes[0]->setDecimals(2);
        spinBoxes[1]->setDecimals(2);
        spinBoxes[2]->setDecimals(2);
        spinBoxes[3]->setDecimals(2);
        spinBoxes[4]->setDecimals(2);
        spinBoxes[5]->setDecimals(2);

        spinBoxes[0]->setMinimum(-10000);
        spinBoxes[1]->setMinimum(-10000);
        spinBoxes[2]->setMinimum(-10000);
        spinBoxes[3]->setMinimum(0);
        spinBoxes[4]->setMinimum(0);
        spinBoxes[5]->setMinimum(0);

        spinBoxes[0]->setMaximum(10000);
        spinBoxes[1]->setMaximum(10000);
        spinBoxes[2]->setMaximum(10000);
        spinBoxes[3]->setMaximum(360);
        spinBoxes[4]->setMaximum(360);
        spinBoxes[5]->setMaximum(360);

        spinBoxes[0]->setSingleStep(5.0);
        spinBoxes[1]->setSingleStep(5.0);
        spinBoxes[2]->setSingleStep(25.0);
        spinBoxes[3]->setSingleStep(1.0);
        spinBoxes[4]->setSingleStep(1.0);
        spinBoxes[5]->setSingleStep(1.0);

        spinBoxes[0]->setValue(0.0);
        spinBoxes[1]->setValue(0.0);
        spinBoxes[2]->setValue(0.0);
        spinBoxes[3]->setValue(0.0);
        spinBoxes[4]->setValue(0.0);
        spinBoxes[5]->setValue(0.0);

        spinBoxes[3]->setWrapping(true);
        spinBoxes[4]->setWrapping(true);
        spinBoxes[5]->setWrapping(true);

        buttons.push_back( new Button( this ) );
        buttons[0]->setText( "reset" );
        buttons[0]->setColorSlot( QColor("black") );

        layout = new QGridLayout( this );
        layout->addWidget(    labels[0],  0,  0 );
        layout->addWidget(    labels[1],  0,  2 );
        layout->addWidget(    labels[2],  0,  4 );
        layout->addWidget(    labels[3],  1,  0 );
        layout->addWidget(    labels[4],  1,  2 );
        layout->addWidget(    labels[5],  1,  4 );
        layout->addWidget( spinBoxes[0],  0,  1 );
        layout->addWidget( spinBoxes[1],  0,  3 );
        layout->addWidget( spinBoxes[2],  0,  5 );
        layout->addWidget( spinBoxes[3],  1,  1 );
        layout->addWidget( spinBoxes[4],  1,  3 );
        layout->addWidget( spinBoxes[5],  1,  5 );
        layout->addWidget(   buttons[0],  2,  0, 1, 6 );

        adjustSize();
    }

    SceneOptions::SceneOptions( QWidget *parent ) : QWidget( parent )
    {
        setWindowTitle("scene options");

        colorDialog = new QColorDialog( this );
        colorDialog->setOption( QColorDialog::ShowAlphaChannel );

        labels.push_back( new QLabel( this ) );
        labels.push_back( new QLabel( this ) );
        labels.push_back( new QLabel( this ) );
        labels.push_back( new QLabel( this ) );
        labels.push_back( new QLabel( this ) );
        labels.push_back( new QLabel( this ) );
        labels.push_back( new QLabel( this ) );
        labels.push_back( new QLabel( this ) );
        labels.push_back( new QLabel( this ) );
        labels.push_back( new QLabel( this ) );
        labels.push_back( new QLabel( this ) );
        labels.push_back( new QLabel( this ) );
        labels.push_back( new QLabel( this ) );

        labels[0]->setText("carriers");
        labels[1]->setText("defects");
        labels[2]->setText("source");
        labels[3]->setText("drain");
        labels[4]->setText("base");
        labels[5]->setText("background");
        labels[6]->setText("sides");
        labels[7]->setText("size");
        labels[8]->setText("size");
        labels[9]->setText("ambient");
        labels[10]->setText("diffuse");
        labels[11]->setText("specular");
        labels[12]->setText("traps");

        labels[0]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        labels[1]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        labels[2]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        labels[3]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        labels[4]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        labels[5]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        labels[6]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        labels[7]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        labels[8]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        labels[9]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        labels[10]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        labels[11]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        labels[12]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

        buttons.push_back( new Button( this ) );
        buttons.push_back( new Button( this ) );
        buttons.push_back( new Button( this ) );
        buttons.push_back( new Button( this ) );
        buttons.push_back( new Button( this ) );
        buttons.push_back( new Button( this ) );
        buttons.push_back( new Button( this ) );
        buttons.push_back( new Button( this ) );
        buttons.push_back( new Button( this ) );
        buttons.push_back( new Button( this ) );

        buttons[0]->setText( "" );
        buttons[1]->setText( "" );
        buttons[2]->setText( "" );
        buttons[3]->setText( "" );
        buttons[4]->setText( "" );
        buttons[5]->setText( "" );
        buttons[6]->setText( "" );
        buttons[7]->setText( "" );
        buttons[8]->setText( "" );
        buttons[9]->setText( "" );

        checkBoxes.push_back( new CheckBox( this ) );
        checkBoxes.push_back( new CheckBox( this ) );
        checkBoxes.push_back( new CheckBox( this ) );
        checkBoxes.push_back( new CheckBox( this ) );
        checkBoxes.push_back( new CheckBox( this ) );
        checkBoxes.push_back( new CheckBox( this ) );
        checkBoxes.push_back( new CheckBox( this ) );
        checkBoxes.push_back( new CheckBox( this ) );
        checkBoxes.push_back( new CheckBox( this ) );
        checkBoxes.push_back( new CheckBox( this ) );
        checkBoxes.push_back( new CheckBox( this ) );
        checkBoxes.push_back( new CheckBox( this ) );

        checkBoxes[0]->setText("hide");
        checkBoxes[1]->setText("hide");
        checkBoxes[2]->setText("hide");
        checkBoxes[3]->setText("hide");
        checkBoxes[4]->setText("hide");
        checkBoxes[5]->setText("hide");
        checkBoxes[6]->setText("spheres");
        checkBoxes[7]->setText("spheres");
        checkBoxes[8]->setText("hide");
        checkBoxes[9]->setText("hide");
        checkBoxes[10]->setText("hide");
        checkBoxes[11]->setText("hide");

        checkBoxes[6]->setChecked(Qt::Unchecked);
        checkBoxes[7]->setChecked(Qt::Unchecked);
        checkBoxes[11]->setChecked(Qt::Checked);

        spinBoxes.push_back( new DSpinBox( this ) );
        spinBoxes.push_back( new DSpinBox( this ) );

        spinBoxes[0]->setDecimals(2);
        spinBoxes[1]->setDecimals(2);

        spinBoxes[0]->setMinimum(0);
        spinBoxes[1]->setMinimum(0);

        spinBoxes[0]->setMaximum(100.0);
        spinBoxes[1]->setMaximum(100.0);

        spinBoxes[0]->setSingleStep(0.1);
        spinBoxes[1]->setSingleStep(0.1);

        spinBoxes[0]->setValue(0.5);
        spinBoxes[1]->setValue(0.5);

        spinBoxes[0]->setWrapping(true);
        spinBoxes[1]->setWrapping(true);

        layout = new QGridLayout( this );
        layout->addWidget(     labels[0],  0,  0 );
        layout->addWidget(     labels[1],  1,  0 );
        layout->addWidget(     labels[2],  2,  0 );
        layout->addWidget(     labels[3],  3,  0 );
        layout->addWidget(     labels[4],  4,  0 );
        layout->addWidget(     labels[5],  6,  0 );
        layout->addWidget(     labels[6],  5,  0 );
        layout->addWidget(     labels[7],  0,  4 );
        layout->addWidget(     labels[8],  1,  4 );
        layout->addWidget(     labels[9],  7,  0 );
        layout->addWidget(    labels[10],  8,  0 );
        layout->addWidget(    labels[11],  9,  0 );
        layout->addWidget(    labels[12], 10,  0 );
        layout->addWidget(    buttons[0],  0,  1 );
        layout->addWidget(    buttons[1],  1,  1 );
        layout->addWidget(    buttons[2],  2,  1 );
        layout->addWidget(    buttons[3],  3,  1 );
        layout->addWidget(    buttons[4],  4,  1 );
        layout->addWidget(    buttons[6],  5,  1 );
        layout->addWidget(    buttons[5],  6,  1 );
        layout->addWidget(    buttons[7],  7,  1 );
        layout->addWidget(    buttons[8],  8,  1 );
        layout->addWidget(    buttons[9],  9,  1 );
        layout->addWidget( checkBoxes[0],  0,  2 );
        layout->addWidget( checkBoxes[1],  1,  2 );
        layout->addWidget( checkBoxes[2],  2,  2 );
        layout->addWidget( checkBoxes[3],  3,  2 );
        layout->addWidget( checkBoxes[4],  4,  2 );
        layout->addWidget( checkBoxes[5],  5,  2 );
        layout->addWidget( checkBoxes[8],  7,  2 );
        layout->addWidget( checkBoxes[9],  8,  2 );
        layout->addWidget(checkBoxes[10],  9,  2 );
        layout->addWidget(checkBoxes[11], 10,  2 );
        layout->addWidget( checkBoxes[6],  0,  3 );
        layout->addWidget( checkBoxes[7],  1,  3 );
        layout->addWidget(  spinBoxes[0],  0,  5 );
        layout->addWidget(  spinBoxes[1],  1,  5 );

        adjustSize();

    }

    RecordDialog::RecordDialog( QWidget *parent ) : QDialog( parent )
    {
        setObjectName("recordDialog");
        adjustSize();
        setWindowTitle("record options");
        setModal(true);

        L1 = new QLabel(this);
        L2 = new QLabel(this);
        L4 = new QLabel(this);
        L5 = new QLabel(this);
        L6 = new QLabel(this);

        L1->setText("work directory:");
        L2->setText("filename stub:");
        L6->setText("type:");
        L4->setText("index start:");
        L5->setText("quality (1-100):");

        L1->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        L2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        L4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        L5->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        L6->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        LE1 = new QLineEdit(this);
        LE1->setObjectName(QString::fromUtf8("LE1"));
        LE1->setText("frame");

        SB2 = new SSpinBox(this);
        SB3 = new SSpinBox(this);

        SB2->setMinimum(0);
        SB3->setMinimum(-1);

        SB2->setMaximum(10000000);
        SB3->setMaximum(100);

        PB1 = new Button(this);
        PB1->setText(QDir::currentPath());

        QStringList formats;
        foreach( QByteArray bin, QImageWriter::supportedImageFormats() )
        {
            QString type = QString("%1").arg( bin.toLower().data() );
            if ( ! formats.contains( type ) )
            {
                formats << type;
            }
        }

        CB1 = new QComboBox(this);
        CB1->clear();
        CB1->insertItems( 0, formats );

        OK = new QDialogButtonBox(this);
        OK->setObjectName(QString::fromUtf8("OK"));
        OK->setOrientation(Qt::Horizontal);
        OK->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        OK->setCenterButtons(false);

        connect(OK, SIGNAL(accepted()), this, SLOT(accept()));
        connect(OK, SIGNAL(rejected()), this, SLOT(reject()));

        connect( PB1, SIGNAL( clicked() ), this, SLOT( openFileDialog() ) );
        connect( LE1, SIGNAL( textChanged( QString ) ), this, SLOT( setStub( QString ) ) );
        connect( SB2, SIGNAL( valueChanged( int ) ), this, SLOT( setCount( int ) ) );
        connect( SB3, SIGNAL( valueChanged( int ) ), this, SLOT( setQuality( int ) ) );
        connect( CB1, SIGNAL( currentIndexChanged( QString ) ), this, SLOT( setType( QString ) ) );

        QMetaObject::connectSlotsByName(this);

        gridLayout = new QGridLayout(this);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->addWidget( L1, 0, 0, 1, 1);
        gridLayout->addWidget( L2, 2, 0, 1, 1);
        gridLayout->addWidget( L4, 3, 0, 1, 1);
        gridLayout->addWidget( L5, 4, 0, 1, 1);
        gridLayout->addWidget( L6, 1, 0, 1, 1);
        gridLayout->addWidget(LE1, 2, 1, 1, 1);
        gridLayout->addWidget(SB2, 3, 1, 1, 1);
        gridLayout->addWidget(PB1, 0, 1, 1, 1);
        gridLayout->addWidget(SB3, 4, 1, 1, 1);
        gridLayout->addWidget(CB1, 1, 1, 1, 1);
        gridLayout->addWidget( OK, 6, 0, 1, 2);

        setWork( QDir::currentPath() );
        setStub( "frame" );
        setCount( 0 );
        setType( "png" );
        setQuality( 100 );
    }

    void RecordDialog::openFileDialog()
    {
        QString dir = QFileDialog::getExistingDirectory(this, "choose work directory", work.absolutePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        setWork( dir );
    }

    void RecordDialog::setWork( QString value )
    {
        QDir newdir = QDir( value );
        if ( newdir.exists() )
        {
            work = QDir( value );
            emit workChanged( value );
        }
        else
        {
            QMessageBox::warning( this, tr( "Langmuir" ), QString("directory %s does not exists!").arg(value));
        }
        PB1->setText( work.absolutePath() );
    }

    void RecordDialog::setStub( QString value )
    {
        stub = value;
        emit stubChanged( value );
        LE1->setText( value );
    }

    void RecordDialog::setType( QString value )
    {
        int index = CB1->findText( value.toLower() );
        if ( index )
        {
            type = CB1->itemText( index );
            emit typeChanged( CB1->itemText( index ) );
            CB1->setCurrentIndex( index );
        }
        else
        {
            QMessageBox::warning( this, tr( "Langmuir" ), QString( "File type %1 not supported!").arg( CB1->itemText( index ) ) );
            CB1->setCurrentIndex( CB1->findText( "png" ) );
        }
    }

    void RecordDialog::setCount( int value )
    {
        count = value;
        emit countChanged( value );
        SB2->setValue( count );
    }

    void RecordDialog::setQuality( int value )
    {
        quality = value;
        emit qualityChanged( value );
        SB3->setValue( quality );
    }

    void GridViewGL::screenShot()
    {
        bool resume = false;
        if ( ! pause )
        {
            togglePauseStatus();
            resume = true;
        }

        QStringList formats;
        QString filter = QString("%1 (*.%1)").arg( QImageWriter::supportedImageFormats()[0].toLower().data() );
        foreach( QByteArray bin, QImageWriter::supportedImageFormats() )
        {
            QString type = QString("%1 (*.%1)").arg( bin.toLower().data() );
            if ( QString( bin.data() ).toLower().trimmed() == "png" ) filter = type;
            if ( ! formats.contains( type ) )
            {
                formats << type;
            }
        }

        QString stub = "untitled";
        openFileDialog:
        QString fileName = QFileDialog::getSaveFileName( this, tr("Save Screen Shot"), QDir::current().absoluteFilePath(stub), tr( qPrintable( formats.join(";;") ) ), &filter );
        QString fileSuf1 = QFileInfo( fileName ).suffix();
        QString fileSuf2 = filter.split("(", QString::SkipEmptyParts)[1].remove(")").remove(";").remove(".").remove("*");

        if ( ! fileName.isEmpty() )
        {
            if ( fileSuf1.isEmpty() )
            {
                fileName = QFileInfo(fileName).baseName() + "." + fileSuf2;
            }
            else
            {
                if ( fileSuf1 != fileSuf2 )
                {
                    QMessageBox msgBox;
                    msgBox.setText(QString("The encoding chosen is %1, but the file suffix is %2.  Note that the encoding will remain %1 unless you choose a new file name..")
                     .arg(fileSuf2)
                     .arg(fileSuf1));
                    msgBox.setInformativeText(QString("Save as?"));
                    msgBox.setStandardButtons(QMessageBox::Cancel);
                    msgBox.setIcon(QMessageBox::Question);
                    QPushButton *option4 = msgBox.addButton( "choose new file name", QMessageBox::ApplyRole );
                    QPushButton *option3 = msgBox.addButton( QFileInfo(fileName).fileName(), QMessageBox::ApplyRole );
                    QPushButton *option2 = msgBox.addButton( QFileInfo(fileName).fileName()+"."+fileSuf2, QMessageBox::ApplyRole );
                    QPushButton *option1 = msgBox.addButton( QFileInfo(fileName).baseName()+"."+fileSuf2, QMessageBox::ApplyRole );
                    msgBox.setDefaultButton( option1 );
                    if ( msgBox.exec() == QMessageBox::Cancel )
                    {
                        return;
                    }
                    if ( msgBox.clickedButton() == option1    )
                    {
                        fileName = QFileInfo(fileName).baseName()+"."+fileSuf2;
                    }
                    if ( msgBox.clickedButton() == option2    )
                    {
                        fileName = QFileInfo(fileName).fileName()+"."+fileSuf2;
                    }
                    if ( msgBox.clickedButton() == option3    )
                    {
                        fileName = QFileInfo(fileName).fileName();
                    }
                    if ( msgBox.clickedButton() == option4    )
                    {
                        stub = QFileInfo(fileName).baseName()+"."+fileSuf2;
                        goto openFileDialog;
                    }
                }
            }

            if ( !(grabFrameBuffer()).save( fileName, qPrintable( fileSuf2 ), 100 ) )
            {
                QMessageBox::warning( this, tr( "Langmuir" ), tr( "Cannot save file %1.\nThe simulation is now paused." ).arg( fileName ) );
            }
            else
            {
                emit statusMessage( QString("snap shot! frame saved to %1").arg(fileName) );
                if ( resume )
                {
                    togglePauseStatus();
                }
            }

        }

    }

    void GridViewGL::timerRecordShot()
    {
        bool resume = false;
        if ( ! pause )
        {
            togglePauseStatus();
            resume = true;
        }

        QString fileName = recordDialog->work.absoluteFilePath( QString("%1%2.%3").arg( recordDialog->stub ).arg( recordDialog->count ).arg( recordDialog->type ) );
        if ( !(grabFrameBuffer()).save( fileName, qPrintable( (recordDialog->type).toUpper() ), recordDialog->quality ) )
        {
            QMessageBox::warning( this, tr( "Langmuir" ), tr( "Cannot save file %1.  Recording is now disabled." ).arg( fileName ) );
            if ( recordTimer ) toggleRecording();
        }
        else
        {
            emit statusMessage( QString("recording! frame saved to %1").arg(fileName), 100 );
            recordDialog->count += 1;
        }

        if ( resume ) togglePauseStatus();
    }

    void GridViewGL::toggleRecording()
    {

        bool resume = false;
        if ( ! pause )
        {
            togglePauseStatus();
            resume = true;
        }

        if ( recording )
        {
            recording = false;
            emit recordChanged( "record" );
            emit recordChangedColor( QColor(255,0,0,255) );
            emit statusMessage( QString("recording stopped, files saved in: %1").arg(recordDialog->work.absolutePath()) );
            return;
        }

        recordDialog->setWork( recordDialog->work.absolutePath() );
        recordDialog->setStub( recordDialog->stub );
        recordDialog->setCount( recordDialog->count );
        recordDialog->setType( recordDialog->type );
        recordDialog->setQuality( recordDialog->quality );
        if ( recordDialog->exec() == QDialog::Accepted )
        {
            emit recordChanged( "stop" );
            emit recordChangedColor( QColor(0,0,255,255) );
            recording = true;
        }

        if ( resume ) togglePauseStatus();

    }

    void DSpinBox::setValueSlot( double value )
    {
        this->setValue( value );
    }

    void SSpinBox::setValueSlot( int value )
    {
        this->setValue( value );
    }

    void CheckBox::setValueSlot( int checkState )
    {
        this->setCheckState( Qt::CheckState( checkState ) );
    }

    void Button::setTextSlot( QString value )
    {
        this->setText( value );
    }

    void Button::setColorSlot( QColor color )
    {
        setStyleSheet(
            QString( "QPushButton {" )
            +QString( "background: qlineargradient( x1: 0, y1: 0, x2: 1, y2: 1, " )
            +QString( "stop: 0.00 %1, ").arg( QColor( 255, 255, 255, 255 ).name() )
            +QString( "stop: 0.20 %1, ").arg( color.name() )
            +QString( "stop: 0.90 %1  ").arg( QColor(  25,  25,  25, 255 ).name() )
            +QString( "); } " )
            +QString( "QPushButton {" )
            +QString( "color: white;" )
            +QString( "}" )
        );
    }

    ColoredObject::ColoredObject( QObject *parent )
        : QObject( parent ), dialog( NULL )
    {
        light.resize(4);
        setColor( color );
        setInvisible( false );
    }

    const QColor& ColoredObject::getColor() const
    {
        return color;
    }

    const float* ColoredObject::getLight() const
    {
        return light.constData();
    }

    void ColoredObject::setColor( QColor color )
    {
        this->color = color;
        if ( invisible )
        {
            setInvisible( Qt::Checked );
        }
        else
        {
            setInvisible( Qt::Unchecked );
        }
        emit colorChanged( color );
        if ( dialog )
        {
            dialog->disconnect();
        }
    }

    void ColoredObject::setInvisible( int checkState )
    {
        switch ( checkState )
        {
            case Qt::Checked :
                this->invisible = true;
                light[0] = 0;
                light[1] = 0;
                light[2] = 0;
                light[3] = 0;
                break;
            default :
                this->invisible = false;
                light[0] = color.redF();
                light[1] = color.greenF();
                light[2] = color.blueF();
                light[3] = color.alphaF();
                break;
        }
    }

    void ColoredObject::setColorDialog( QColorDialog *dialog )
    {
        this->dialog = dialog;
    }

    void ColoredObject::openColorDialog()
    {
        if ( dialog )
        {
            dialog->setCurrentColor( color );
            connect( dialog, SIGNAL( colorSelected( QColor ) ), this, SLOT( setColor( QColor ) ) );
            dialog->exec();
        }
        else
        {
            QMessageBox::warning( 0, "Langmuir", "no color dialog set for object!" );
        }
    }

    Controls::Controls( QWidget *parent ) : QWidget( parent )
    {
        setWindowTitle("controls");

        labels.push_back( new QLabel( this ) );
        labels.push_back( new QLabel( this ) );
        labels.push_back( new QLabel( this ) );
        labels.push_back( new QLabel( this ) );
        labels.push_back( new QLabel( this ) );

        labels[0]->setText("updateGL");
        labels[1]->setText("iterations");
        labels[2]->setText("step");

        labels[0]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        labels[1]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        labels[2]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

        buttons.push_back( new Button( this ) );
        buttons.push_back( new Button( this ) );
        buttons.push_back( new Button( this ) );

        buttons[0]->setText( "play" );
        buttons[1]->setText( "record" );
        buttons[2]->setText( "snap shot" );

        buttons[0]->setColorSlot( QColor( 12, 127, 9 ) );
        buttons[1]->setColorSlot( QColor( 255, 0, 0 ) );
        buttons[2]->setColorSlot( QColor( 255, 255, 0 ) );

        checkBoxes.push_back( new CheckBox( this ) );
        checkBoxes.push_back( new CheckBox( this ) );

        checkBoxes[0]->setText("openCL");
        checkBoxes[1]->setText("coulomb");

        checkBoxes[0]->setChecked(Qt::Checked);
        checkBoxes[1]->setChecked(Qt::Checked);

        lcdNumbers.push_back( new QLCDNumber( this ) );
        lcdNumbers[0]->setLayoutDirection(Qt::LeftToRight);
        lcdNumbers[0]->setFrameShape(QFrame::Panel);
        lcdNumbers[0]->setFrameShadow(QFrame::Raised);
        lcdNumbers[0]->setDigitCount(15);
        lcdNumbers[0]->setProperty("value", QVariant(0));

        spinBoxes.push_back( new SSpinBox( this ) );
        spinBoxes.push_back( new SSpinBox( this ) );

        spinBoxes[0]->setMinimum(1);
        spinBoxes[1]->setMinimum(1);

        spinBoxes[0]->setMaximum(100);
        spinBoxes[1]->setMaximum(100);

        spinBoxes[0]->setValue(100);
        spinBoxes[1]->setValue(100);

        spinBoxes[0]->setWrapping(true);
        spinBoxes[1]->setWrapping(true);

        layout = new QGridLayout( this );
        layout->addWidget(     labels[0],  1,  0 );
        layout->addWidget(     labels[1],  2,  0 );
        layout->addWidget(     labels[2],  3,  0 );
        layout->addWidget(    buttons[0],  0,  0,  1,  2 );
        layout->addWidget(    buttons[1],  0,  2 );
        layout->addWidget(    buttons[2],  3,  2 );
        layout->addWidget(  spinBoxes[0],  1,  1 );
        layout->addWidget(  spinBoxes[1],  2,  1 );
        layout->addWidget( checkBoxes[0],  1,  2 );
        layout->addWidget( checkBoxes[1],  2,  2 );
        layout->addWidget( lcdNumbers[0],  3,  1,  1,  1 );

        adjustSize();
    }

}
