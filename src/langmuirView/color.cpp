#include "color.h"

namespace color {
    void glColor3f(QColor& color)
    {

    }

    void glColor4f(QColor& color)
    {

    }

    float* qColorToArray4(const QColor &color, float *array)
    {
        array[0] = color.redF();
        array[1] = color.greenF();
        array[2] = color.blueF();
        array[3] = color.alphaF();
        return array;
    }

    float* qColorToArray4(const QColor &color)
    {
        static float array[4];
        qColorToArray4(color, array);
        return array;
    }
}
