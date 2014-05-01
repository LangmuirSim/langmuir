#ifndef COLOR_H
#define COLOR_H

#include <QObject>
#include <QColor>

template <class T> void qColorToArray3(const QColor& color, T *array);
template <class T> void qColorToArray4(const QColor& color, T *array);

template <> inline void qColorToArray3(const QColor& color, float *array)
{
    array[0] = color.redF();
    array[1] = color.greenF();
    array[2] = color.blueF();
}

template <> inline void qColorToArray4(const QColor& color, float *array)
{
    qColorToArray3<float>(color, array);
    array[3] = color.alphaF();
}

template <> inline void qColorToArray3(const QColor& color, double *array)
{
    array[0] = color.redF();
    array[1] = color.greenF();
    array[2] = color.blueF();
}

template <> inline void qColorToArray4(const QColor& color, double *array)
{
    qColorToArray3<double>(color, array);
    array[3] = color.alphaF();
}

template <> inline void qColorToArray3(const QColor& color, int *array)
{
    array[0] = color.red();
    array[1] = color.green();
    array[2] = color.blue();
}

template <> inline void qColorToArray4(const QColor& color, int *array)
{
    qColorToArray3<int>(color, array);
    array[3] = color.alpha();
}

#endif // COLOR_H
