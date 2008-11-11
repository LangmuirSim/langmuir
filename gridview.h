#ifndef GRIDDISPLAY_H
#define GRIDDISPLAY_H

#include <QDialog>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QObject>
#include <QVector>

namespace Langmuir
{
  class Grid;

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
    GridItem(int type = 0, unsigned long int site = 0);
    ~GridItem();
    void setCharge(int charge);
    int charge() { return m_charge; }
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget);

  private:
    int m_type;
    unsigned long int m_site;
    int m_charge;
    QColor* m_color;
    int m_width, m_height;
  };

  class GridScene : public QGraphicsScene
  {
    Q_OBJECT

  public:
    GridScene(unsigned long width = 0, unsigned long height = 0,
              QObject* parent = 0);
    ~GridScene();

  private:
    Grid *m_grid;
    QVector<int> *m_sites;
  };
}

#endif /*GRIDDISPLAY_H */
