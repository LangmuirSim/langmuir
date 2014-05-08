#ifndef BOX_H
#define BOX_H

#include "sceneobject.h"

/**
 * @brief A class to represent a textured box
 */
class Box : public SceneObject
{
    Q_OBJECT
public:
    /**
     * @brief create the Box
     * @param viewer the viewer
     * @param parent QObject this belongs to
     */
    explicit Box(LangmuirViewer &viewer, QObject *parent = 0);

    /**
      * @brief destroy the Box
      */
    ~Box();

    /**
     * @brief the faces to show texture on
     */
    enum Face {
        None  =  1, //! do not show texture on any face
        North =  2, //! show texture on the +y
        South =  4, //! show texture on the -y
        East  =  8, //! show texture on the +x
        West  = 16, //! show texture on the -x
        Front = 32, //! show texture on the +z
        Back  = 64, //! show texture on the -z
        All   = North | South | East | West | Front | Back //! show the texture on all faces
    };
    Q_DECLARE_FLAGS(Faces, Face)
    Q_FLAGS(Faces)

signals:
    /**
     * @brief signal that the color of has changed
     * @param color value of color
     */
    void colorChanged(QColor color);

    /**
     * @brief signal that the x length has changed
     * @param value value of length
     */
    void xSizeChanged(double value);

    /**
     * @brief signal that the y length has changed
     * @param value value of length
     */
    void ySizeChanged(double value);

    /**
     * @brief signal that the z length has changed
     * @param value value of length
     */
    void zSizeChanged(double value);

    /**
     * @brief signal that the texture is drawn changed
     * @param drawn true if texture is drawn
     */
    void imageOnChanged(bool drawn);

    /**
     * @brief signal that which faces the texture appears on changed
     * @param faces list of faces
     */
    void facesChanged(Faces faces);

public slots:
    /**
     * @brief make signal/slot connections
     */
    virtual void makeConnections();

    /**
     * @brief set the color
     * @param color color to set
     */
    void setColor(QColor color);

    /**
     * @brief set the x length
     * @param value value to set
     */
    void setXSize(double value);

    /**
     * @brief set the y length
     * @param value value to set
     */
    void setYSize(double value);

    /**
     * @brief set the z length
     * @param value value to set
     */
    void setZSize(double value);

    /**
     * @brief set the list of faces to show texture on
     * @param faces list of faces
     */
    void setFaces(Faces faces);

    /**
     * @brief load the texture
     * @param image image to use for texture
     */
    void loadImage(const QImage &image);

    /**
     * @brief show the texture
     * @param on true if texture is to be shown
     */
    void showImage(bool on=true);

    /**
     * @brief toggle if texture is shown
     */
    void toggleImage();

protected:
    /**
     * @brief initialize object
     */
    virtual void init();

    /**
     * @brief perform OpenGL drawing operations
     */
    virtual void draw();

    //! color of box
    QColor m_color;

    //! x length
    double m_xsize;

    //! y length
    double m_ysize;

    //! z length
    double m_zsize;

    //! texture ID
    GLuint m_imageID;

    //! show texture
    bool   m_imageOn;

    //! texture faces
    Faces  m_faces;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Box::Faces)

#endif // BOX_H
