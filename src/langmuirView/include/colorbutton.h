#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QColorDialog>
#include <QPushButton>
#include <QColor>
#include <QDebug>

namespace LangmuirView {

/**
 * @brief A class to open color dialogs by clicking.
 */
class ColorButton : public QPushButton
{
    Q_OBJECT
public:
    /**
     * @brief create the ColorButton
     * @param parent QObject this belongs to
     */
    explicit ColorButton(QWidget *parent = 0);

    /**
     * @brief Get a reference to the dialog
     * @return dialog
     */
    QColorDialog& colorDialog();

    /**
      * @brief destroy the ColorButton
      */
    ~ColorButton();

signals:
    /**
     * @brief signal that the color of has changed
     * @param color value of color
     */
    void selectedColor(QColor color);

public slots:
    /**
     * @brief set the current color
     * @param color value of color to set
     */
    void setButtonColor(QColor color);

    /**
     * @brief open dialog and get user selected color
     */
    void getColor();

protected:
    //! static dialog
    static QColorDialog *m_colordialog;

    //! current color
    QColor m_color;
};

}

#endif // COLORBUTTON_H
