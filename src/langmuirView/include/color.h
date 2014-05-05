#ifndef COLOR_H
#define COLOR_H

#include <QColor>

namespace color {
    /**
     * @brief Wrapper around glColor3f using QColor
     */
    void glColor3f(QColor& color);

    /**
     * @brief Wrapper around glColor4f using QColor
     */
    void glColor4f(QColor& color);

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
