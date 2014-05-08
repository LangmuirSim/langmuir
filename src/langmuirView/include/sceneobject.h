#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <QVector>
#include <QObject>
#include <QColor>
#include <QDebug>

#ifdef Q_OS_MAC
    #include <OpenGL/glu.h>
#else
    #include <GL/glu.h>
#endif

class LangmuirViewer;

/**
 * @brief Base class for objects in OpenGL scene
 */
class SceneObject : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief create the SceneObject
     * @param viewer the viewer
     * @param parent QObject this belongs to
     */
    explicit SceneObject(LangmuirViewer &viewer, QObject *parent = 0);

    /**
     * @brief true if object is drawn
     */
    bool isVisible();

    /**
     * @brief calls OpenGL drawing commands.
     *
     * Use this inside the paintGL() or draw() functions of the main OpenGL widget.
     */
    void render();

signals:
    /**
     * @brief signal that the visibility has changed
     * @param drawn true if object is visible
     */
    void visibleChanged(bool drawn);

public slots:
    /**
     * @brief toggle visibility
     */
    void toggleVisible();

    /**
     * @brief set the visibility
     * @param draw true if object is to be drawn
     */
    void setVisible(bool draw = true);

    /**
     * @brief make signal/slot connections
     */
    virtual void makeConnections();

protected:
    /**
     * @brief initialize object
     *
     * Explicitly call this in derived class constructor.
     */
    virtual void init();

    /**
     * @brief perform OpenGL drawing operations
     */
    virtual void draw();

    /**
     * @brief perform OpenGL drawing operations before draw()
     */
    virtual void preDraw();

    /**
     * @brief perform OpenGL drawing operations after draw()
     */
    virtual void postDraw();

    //! reference to OpenGL widget
    LangmuirViewer &m_viewer;

    //! visibility
    bool visible_;
};

#endif // SCENEOBJECT_H
