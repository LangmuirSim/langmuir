

#include "gridview.h"

#include <QtGui>

using namespace Langmuir;

int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  GridScene scene(100, 600);
  scene.setSceneRect(0, 0, 100, 600);
  scene.setItemIndexMethod(QGraphicsScene::NoIndex);

  GridView view(&scene);
//  view.setRenderHint(QPainter::Antialiasing);
//  view.setCacheMode(QGraphicsView::CacheBackground);
  view.setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
  view.setDragMode(QGraphicsView::ScrollHandDrag);
  view.setWindowTitle(QT_TRANSLATE_NOOP(QGraphicsView, "Langmuir Grid View"));
  view.resize(110, 610);
  view.show();

  return app.exec();
 }