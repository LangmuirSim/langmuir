#ifndef GRIDDISPLAY_H_
#define GRIDDISPLAY_H_

#include <QDialog>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QObject>

namespace Langmuir
{

  class GridView : public QGraphicsView
  {
    Q_OBJECT

  public:
    GridView(QWidget* parent = 0);
    explicit GridView(QGraphicsScene* scene, QWidget* parent = 0);

  };

  class GridItem : public QObject, public QGraphicsItem
  {
    Q_OBJECT

  public:
    GridItem(int type = 0);
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget);

  private:
    int m_type;
    QColor* m_color;
    int m_width, m_height;
  };

  class GridScene : public QGraphicsScene
  {
    Q_OBJECT

  public:
    GridScene(QObject* parent = 0);
  };
}

#endif /*GRIDDISPLAY_H_*/
