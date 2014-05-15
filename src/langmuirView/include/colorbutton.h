#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QColorDialog>
#include <QPushButton>
#include <QDebug>

class ColorButton : public QPushButton
{
    Q_OBJECT
public:
    explicit ColorButton(QWidget *parent = 0);
    ~ColorButton();

signals:
    void selectedColor(QColor color);

public slots:
    void setButtonColor(QColor color);
    void getColor();

protected:
    static QColorDialog *m_colordialog;
};

#endif // COLORBUTTON_H
