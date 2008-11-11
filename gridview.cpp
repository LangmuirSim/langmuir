#include "gridview.h"

#include "cubicgrid.h"
#include "rand.h"

#include <QDebug>

namespace Langmuir
{

  /**
   * GridView - gives the actual widget that is visible
   */

  GridView::GridView(QWidget* parent) : QGraphicsView(parent)
  {
    setWindowFlags(Qt::Dialog | Qt::Tool);

  }

  GridView::GridView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
  {
    setWindowFlags(Qt::Dialog | Qt::Tool);
  }

  /**
   * GridItem - paints the individual grid items
   */

  GridItem::GridItem(int type, unsigned long int site) : m_type(type),
    m_site(site), m_charge(0), m_color(new QColor(240, 240, 240)), m_width(1),
    m_height(1)
  {
  }

  GridItem::~GridItem()
  {
    delete m_color;
  }

  void GridItem::setCharge(int charge)
  {
    m_charge = charge;
//    qDebug() << "Charge set at" << charge;
    if (m_charge == -1) {
      *m_color = QColor(255, 0, 0);
    }
    else if (m_charge == 1) {
      *m_color = QColor(0, 0, 255);
    }
  }

  QRectF GridItem::boundingRect() const
  {
    return QRectF(-m_width/2, -m_height/2, m_width, m_height);
  }

  QPainterPath GridItem::shape() const
  {
    QPainterPath path;
    path.addRect(-m_width/2, -m_height/2, m_width, m_height);
    return path;
  }

  void GridItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                       QWidget *widget)
  {
    painter->setBrush(*m_color);
    painter->setPen(*m_color);
    QRectF rect(-m_width/2, -m_height/2, m_width, m_height);
    painter->drawRect(rect);
  }

  /**
   * GridScene - where everything is stored.
   */
  GridScene::GridScene(unsigned long width, unsigned long height,
    QObject *parent) : QGraphicsScene(parent)
  {
    m_grid = new CubicGrid(width, height);

    Rand rnd(0, 1.1);

    GridItem *item = 0;
    for (unsigned long i = 0; i < width; ++i) {
      for (unsigned long j = 0; j < height; ++j) {
        item = new GridItem(0);
        item->setPos(i, j);
        item->setCharge(rnd.number());
        addItem(item);
      }
    }
  }

  GridScene::~GridScene()
  {
    delete m_grid;
  }

}

#include "gridview.moc"
