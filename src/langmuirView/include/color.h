#ifndef COLOR_H
#define COLOR_H

#include <QObject>
#include <QColor>

class LangmuirViewer;

namespace color {

    /**
     * @brief Copy color data to array of size 4
     */
    float* qColorToArray4(const QColor &color, float *array);

    /**
     * @brief Copy color data to array of size 4 (static)
     */
    float* qColorToArray4(const QColor &color);
}

#endif // COLOR_H
