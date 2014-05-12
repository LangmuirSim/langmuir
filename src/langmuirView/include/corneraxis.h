#ifndef CORNERAXIS_H
#define CORNERAXIS_H

#include "axis.h"

/**
 * @brief A class to represent an xyz axis that doesnt change size/position
 */
class CornerAxis : public Axis
{
    Q_OBJECT
    Q_ENUMS(Location)

public:
    /**
     * @brief the location of the axis
     */
    enum Location {
        LowerLeft,   //! draw axis in lower left
        UpperLeft,   //! draw axis in upper left
        LowerRight,  //! draw axis in lower right
        UpperRight   //! draw axis in upper right
    };

    /**
     * @brief create the CornerAxis
     * @param viewer the viewer
     * @param parent QObject this belongs to
     */
    explicit CornerAxis(LangmuirViewer &viewer, QObject *parent = 0);

    /**
     * @brief get the location of the corner axis
     */
    Location getLocation() const;

    /**
     * @brief get the size of the viewport
     */
    int getSize() const;

    /**
     * @brief get the shift of the viewport
     */
    int getShift() const;

signals:
    /**
     * @brief signal that the axis location has changed
     */
    void locationChanged(Location);

    /**
     * @brief signal that the axis shift has changed
     */
    void shiftChanged(int);

    /**
     * @brief signal that the axis size has changed
     */
    void sizeChanged(int);

public slots:
    /**
     * @brief set the axis location
     * @param location location to set
     */
    void setLocation(Location location);

    /**
     * @brief set the axis shift
     * @param shift shift to set
     */
    void setShift(int shift);

    /**
     * @brief set the axis size
     * @param size size to set
     */
    void setSize(int size);

    /**
     * @brief make signal/slot connections
     */
    virtual void makeConnections();

protected:
    /**
     * @brief initialize object
     */
    virtual void init();

    /**
     * @brief perform OpenGL drawing operations before draw()
     */
    virtual void preDraw();

    /**
     * @brief perform OpenGL drawing operations after draw()
     */
    virtual void postDraw();

private:
    //! location of axis
    Location m_location;

    //! storage for scissor box
    int m_scissorBox[4];

    //! storage for viewport
    int m_viewPort[4];

    //! shift of axis from edge
    int m_shift;

    //! size of altered viewport
    int m_size;
};

#endif // CORNERAXIS_H
