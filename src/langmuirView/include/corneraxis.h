#ifndef CORNERAXIS_H
#define CORNERAXIS_H

#include "axis.h"

class CornerAxis : public Axis
{
    Q_OBJECT
    Q_ENUMS(Location)

public:
    enum Location {
        LowerLeft,
        UpperLeft,
        LowerRight,
        UpperRight
    };

    explicit CornerAxis(LangmuirViewer &viewer, QObject *parent = 0);

signals:
    void locationChanged(Location);
    void shiftChanged(int);
    void sizeChanged(int);

public slots:
    void setLocation(Location location);
    void setShift(int shift);
    void setSize(int size);

protected:
    virtual void init();
    virtual void preDraw();
    virtual void postDraw();

private:
    Location m_location;
    int m_scissorBox[4];
    int m_viewPort[4];
    int m_shift;
    int m_size;
};

#endif // CORNERAXIS_H
