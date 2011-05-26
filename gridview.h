#ifndef GRID_VIEW_H
#define GRID_VIEW_H

#include <GL/glew.h>
#include "inputparser.h"
#include "cubicgrid.h"
#include "world.h"
#include "simulation.h"
#include "chargeagent.h"
#include "cubicgrid.h"

#include <QtCore>
#include <QtGui>
#include <QGLWidget>
#include <QMatrix4x4>

namespace Langmuir
{

  class ColoredObject : public QObject
  {
  Q_OBJECT public:
   ColoredObject( QObject *parent )
    : QObject( parent ), invisible(false) {};
   const QColor& getColor() const { return color; };

   public slots:
    void setColor( QColor color );
    void setInvisible( int checkState );
 
   signals:
    void colorChanged( QColor color );

   protected:
    QColor color;
    bool invisible;
  };

  class SpinBox : public QDoubleSpinBox
  {
  Q_OBJECT public:
   SpinBox( QWidget * parent ) : QDoubleSpinBox( parent ) {};
   public slots:
    void setValueSlot( double value ) { this->setValue( value ); }
  };

  class Button : public QPushButton
  {
  Q_OBJECT public:
   Button( QWidget * parent ) : QPushButton( parent ) {};
   public slots:
    void setTextSlot( QString value ) { this->setText( value ); }
  };

  class ColorButton : public Button
  {
  Q_OBJECT public:
   ColorButton( QWidget * parent, ColoredObject * coloredObject = 0, QColorDialog * colorDialog = 0 )
    : Button( parent ), coloredObject( coloredObject ), colorDialog( colorDialog ) {};
   public slots:
    void openColorDialog();
   private:
    ColoredObject* coloredObject;
    QColorDialog* colorDialog;
  };

  class StayOpen : public QWidget
  {
  Q_OBJECT public:
   StayOpen( QWidget * parent ) : QWidget( parent ) {};
   protected:
    void closeEvent( QCloseEvent *event ) { event->ignore(); }
  };

  class Box : public ColoredObject
  {
  Q_OBJECT public:
   Box( QObject *parent, QVector3D dimensions, QVector3D origin, QColor color );
  ~Box();
   void draw() const;

   private:
    GLuint vVBO;
    GLuint nVBO;
  };

  class PointArray : public ColoredObject
  {
  Q_OBJECT public:
   PointArray( QObject *parent, QVector<float>& xyz, QColor color, float pointsize );
  ~PointArray();
   void draw( int size, int height, float fov ) const;
   void update( QVector<float>& xyz, int size );
   const QColor& getColor() const { return color; };

   public slots:
    void setSpheres( int checkState );
    void setPointSize( double pointSize );

   signals:
    void pointSizeChanged( double pointSize );

   private:
    GLuint vVBO;
    GLuint prog;
    float pointSize;
    bool spheres;
  };

  class GridViewGL : public QGLWidget
  {
  Q_OBJECT public:
    GridViewGL(QWidget * parent, QString input );
   ~GridViewGL();
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

   public slots:
    void setXTranslation( double length );
    void setYTranslation( double length );
    void setZTranslation( double length );
    void setXRotation( double angle );
    void setYRotation( double angle );
    void setZRotation( double angle );
    void setPauseStatus();
    void timerUpdateGL();

   signals:
    void xTranslationChanged( double angle );
    void yTranslationChanged( double angle );
    void zTranslationChanged( double angle );
    void xRotationChanged( double angle );
    void yRotationChanged( double angle );
    void zRotationChanged( double angle );
    void pauseChanged( QString );

   protected:
    void initializeGL();
    void initializeNavigator();
    void initializeSceneOptions();
    void resizeGL(int w, int h);
    void paintGL();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);

   private:
    void NormalizeAngle(double &angle);
    QVector3D translation;
    QVector3D rotation;
    QVector3D delta;
    float thickness;
    float fov;
    bool pause;
    QPoint lastPos;
    QTimer *qtimer;
    Box *base;
    Box *source;
    Box *drain;
    Box *side1;
    Box *side2;
    Box *side3;
    Box *side4;
    Box *side5;
    Box *side6;
    ColoredObject *background;
    QVector<float> pointBuffer;
    PointArray *carriers;
    PointArray *defects;
    InputParser *pInput;
    SimulationParameters *pPar;
    Simulation *pSim;

    StayOpen* navigator;
    QGridLayout* navigatorLayout;
    Button* navigatorPauseButton;
    QList< QLabel* > navigatorLabels;
    QList< SpinBox* > navigatorDSBoxes;

    StayOpen* sceneOptions;
    QColorDialog* sceneOptionsColorDialog;
    QGridLayout* sceneOptionsLayout;
    QList< ColorButton* > sceneOptionsColorButtons;
    QList< QLabel* > sceneOptionsLabels;
    QList< QCheckBox * > sceneOptionsCheckBoxes;
    QList< SpinBox* > sceneOptionsDSBoxes;

  };

  class MainWindow:public QWidget
  {
  Q_OBJECT public:
    MainWindow(QString input);
    GridViewGL *glWidget;
  };

}

#endif
