#include "ImageViewer.hpp"
#include <qabstractscrollarea.h>
#include <qpixmap.h>
#include <qscrollarea.h>
#include <qscrollbar.h>
#include <qsharedpointer.h>

ImageViewer::ImageViewer(QWidget *parent) : QGraphicsView(parent), zoomFactor(1.15) {
  setScene(&scene);
  setDragMode(QGraphicsView::ScrollHandDrag);
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

void ImageViewer::setImage(const QPixmap &pixmap) {
  pixmapItem = scene.addPixmap(pixmap);
  scene.setSceneRect(pixmap.rect());
}

void ImageViewer::useImageTransform(const ImageViewer &other) {
  this->setTransform(other.transform());
  this->horizontalScrollBar()->setValue(other.horizontalScrollBar()->value());
  this->verticalScrollBar()->setValue(other.verticalScrollBar()->value());
}

void ImageViewer::fit() {
  if (pixmapItem) {
    QRectF bounds = pixmapItem->boundingRect();
    scene.setSceneRect(bounds);
    fitInView(bounds, Qt::KeepAspectRatio);
  }
}

void ImageViewer::clear() { scene.clear(); }

QPixmap ImageViewer::getImage() const { return pixmapItem->pixmap(); }

void ImageViewer::wheelEvent(QWheelEvent *event) {
  if (event->angleDelta().y() > 0)
    scale(zoomFactor, zoomFactor); // zoom in
  else
    scale(1 / zoomFactor, 1 / zoomFactor); // zoom out
}
