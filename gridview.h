#ifndef GRID_VIEW_H
#define GRID_VIEW_H

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
#include <QGLBuffer>
#include <QGLShaderProgram>

namespace Langmuir
{
    class ColoredObject : public QObject
    {
        Q_OBJECT public:
            ColoredObject( QObject *parent = 0 );
            const QColor& getColor() const;
            const float* getLight() const;

        public slots:
            void setColor( QColor color );
            void setInvisible( int checkState );
            void setColorDialog( QColorDialog *dialog );
            void openColorDialog();

        signals:
            void colorChanged( QColor color );

        protected:
            QVector<float> light;
            QColorDialog *dialog;
            bool invisible;
            QColor color;
    };

    class Box : public ColoredObject
    {
        Q_OBJECT public:
            Box( QObject *parent, QVector3D dimensions, QVector3D origin );
            ~Box();
            void draw();

        public slots:
            void setTexture( int tid )
            {
                this->tid = tid;
            }

        private:
            QGLBuffer vBuffer;
            QGLBuffer nBuffer;
            QGLBuffer tBuffer;
            int tid;
    };

    class PointArray : public ColoredObject
    {
        Q_OBJECT public:
            PointArray( QObject *parent, QVector<float>& xyz );
            ~PointArray();
            void draw( int size, int height, float fov );
            void update( QVector<float> &xyz, int size );

        public slots:
            void setSpheres( int checkState );
            void setPointSize( double pointSize );

        signals:
            void pointSizeChanged( double pointSize );

        private:
            QGLShaderProgram program;
            QGLBuffer vBuffer;
            float pointSize;
            bool spheres;
    };

    class DSpinBox : public QDoubleSpinBox
    {
        Q_OBJECT public:
            DSpinBox( QWidget * parent ) : QDoubleSpinBox( parent ) {};
        public slots:
            void setValueSlot( double value )
            {
                this->setValue( value );
            }
    };

    class SSpinBox : public QSpinBox
    {
        Q_OBJECT public:
            SSpinBox( QWidget * parent ) : QSpinBox( parent ) {};
        public slots:
            void setValueSlot( int value )
            {
                this->setValue( value );
            }
    };

    class CheckBox : public QCheckBox
    {
        Q_OBJECT public:
            CheckBox( QWidget * parent ) : QCheckBox( parent ) {};
        public slots:
            void setValueSlot( int checkState )
            {
                this->setCheckState( Qt::CheckState( checkState ) );
            }
    };

    class Button : public QPushButton
    {
        Q_OBJECT public:
            Button( QWidget * parent );
        public slots:
            void setTextSlot( QString value )
            {
                this->setText( value );
            }
            void setColorSlot( QColor color );
    };

    class RecordDialog : public QDialog
    {
        Q_OBJECT public:
            RecordDialog( QWidget *parent = 0 );
            QGridLayout *gridLayout;
            QLabel *L1;
            QLabel *L2;
            QLabel *L3;
            QLabel *L4;
            QLabel *L5;
            QLabel *L6;
            QLineEdit *LE1;
            SSpinBox *SB1;
            SSpinBox *SB2;
            SSpinBox *SB3;
            QComboBox *CB1;
            QPushButton *PB1;
            QDialogButtonBox *OK;

        public slots:
            void openFileDialog();
            void setWork( QString );
            void setStub( QString );
            void setType( QString );
            void setRate( int );
            void setCount( int );
            void setQuality( int );

        signals:
            void workChanged( QString value );
            void stubChanged( QString value );
            void typeChanged( QString value );
            void rateChanged( int value );
            void countChanged( int value );
            void qualityChanged( int value );

        public:
            QDir work;
            QString stub;
            QString type;
            int rate;
            int count;
            int quality;
    };

    class GridViewGL : public QGLWidget
    {
        Q_OBJECT public:
            GridViewGL( const QGLFormat &format, QWidget * parent, QString input );
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
            void setTimerInterval( int time );
            void setIterationsPrint( int iterationsPrint );
            void toggleCoulombStatus(int checkState);
            void toggleOpenCLStatus(int checkState);
            void toggleTrapsTexture(int checkState);
            void togglePauseStatus();
            void toggleRecording();
            void timerUpdateGL();
            void timerRecordShot();
            void screenShot();
            QImage drawEnergyLandscape( unsigned int layer = 0 );

        signals:
            void xTranslationChanged( double angle );
            void yTranslationChanged( double angle );
            void zTranslationChanged( double angle );
            void xRotationChanged( double angle );
            void yRotationChanged( double angle );
            void zRotationChanged( double angle );
            void statusMessage( QString, int time = 0 );
            void pauseChanged( QString );
            void recordChanged( QString );
            void recordChangedColor( QColor );
            void openCLStatusChanged( int checkState );
            void coulombStatusChanged( int checkState );
            void iterationsPrintChanged( int iterationsPrint );
            void timerIntervalChanged( int time );
            void stepChanged( int step );
            void currentChanged( double current );
            void initialized();

        protected:
            void initializeGL();
            void resizeGL(int w, int h);
            void paintGL();
            void mousePressEvent(QMouseEvent *event);
            void mouseMoveEvent(QMouseEvent *event);
            void wheelEvent(QWheelEvent *event);
            void keyPressEvent(QKeyEvent *event);

        public:
            void NormalizeAngle(double &angle);
            QVector3D translation;
            QVector3D rotation;
            QVector3D delta;
            float thickness;
            float fov;
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
            ColoredObject *ambientLight;
            ColoredObject *diffuseLight;
            ColoredObject *specularLight;
            ColoredObject *whiteLight;
            ColoredObject *blackLight;
            QVector<float> pointBuffer;
            PointArray *carriers;
            PointArray *defects;
            InputParser *pInput;
            SimulationParameters *pPar;
            Simulation *pSim;
            bool pause;
            QTimer *recordTimer;
            RecordDialog *recordDialog;
            unsigned int lastCount;
            int updateTime;
            int step;
            GLuint trapsTexture;
            GLuint metalTexture;
    };

    class Navigator: public QWidget
    {
        Q_OBJECT public:
            Navigator( QWidget *parent );
            QGridLayout* layout;
            QList< QLabel* > labels;
            QList< DSpinBox* > spinBoxes;
    };

    class SceneOptions: public QWidget
    {
        Q_OBJECT public:
            SceneOptions( QWidget *parent );
            QGridLayout* layout;
            QColorDialog* colorDialog;
            QList< Button* > buttons;
            QList< QLabel* > labels;
            QList< DSpinBox* > spinBoxes;
            QList< CheckBox* > checkBoxes;
    };

    class Controls: public QWidget
    {
        Q_OBJECT public:
            Controls( QWidget *parent );
            QGridLayout* layout;
            QList< Button* > buttons;
            QList< QLabel* > labels;
            QList< SSpinBox* > spinBoxes;
            QList< CheckBox* > checkBoxes;
            QList< QLCDNumber* > lcdNumbers;
    };

    class MainWindow: public QMainWindow
    {
        Q_OBJECT public:
            MainWindow( QString input );
            GridViewGL *glWidget;
            Navigator *navigator;
            SceneOptions *sceneOptions;
            Controls *controls;
        public slots:
            void setConnections();
    };

}

#endif
