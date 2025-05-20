#include "ImageViewer.hpp"
#include <qabstractscrollarea.h>
#include <qgraphicsitem.h>
#include <qnamespace.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qrubberband.h>
#include <qscrollarea.h>
#include <qscrollbar.h>
#include <qsharedpointer.h>
#include <qtpreprocessorsupport.h>
#include <qwindowdefs.h>

ImageViewer::ImageViewer(QWidget *parent) : QGraphicsView(parent), zoomFactor(1.15) {
  setScene(&scene);
  setDragMode(QGraphicsView::ScrollHandDrag);
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
}

void ImageViewer::setImage(const QPixmap &pixmap) {
  scene.clear();
  imageItem = scene.addPixmap(pixmap);
  imageItem->setZValue(0);
  scene.setSceneRect(pixmap.rect());
  cancelGetLineFromUser();
}

void ImageViewer::useImageTransform(const ImageViewer &other) {
  this->setTransform(other.transform());
  this->horizontalScrollBar()->setValue(other.horizontalScrollBar()->value());
  this->verticalScrollBar()->setValue(other.verticalScrollBar()->value());
}

void ImageViewer::fit() {
  if (imageItem) {
    QRectF bounds = imageItem->boundingRect();
    scene.setSceneRect(bounds);
    fitInView(bounds, Qt::KeepAspectRatio);
  }
}

void ImageViewer::clear() { scene.clear(); }

QPixmap ImageViewer::getImage() const { return imageItem->pixmap(); }

void ImageViewer::wheelEvent(QWheelEvent *event) {
  if (event->angleDelta().y() > 0)
    scale(zoomFactor, zoomFactor); // zoom in
  else
    scale(1 / zoomFactor, 1 / zoomFactor); // zoom out
}

QGraphicsEllipseItem *ImageViewer::drawPoint(QPointF point) {
  const double radius = 2;
  QPen pointPen = QPen(Qt::red, 5);
  QBrush pointBrush = QBrush(Qt::SolidPattern);

  QGraphicsEllipseItem *pointItem = scene.addEllipse(
      point.x() - radius, point.y() - radius, radius * 2.0, radius * 2.0, pointPen, pointBrush);
  pointItem->setZValue(1);
  return pointItem;
}

void ImageViewer::mousePressEvent(QMouseEvent *event) {
  if (selectingLine && event->button() == Qt::LeftButton) {
    QPointF scenePos = mapToScene(event->pos());
    QPointF imagePos = imageItem->mapFromScene(scenePos);

    if (firstPoint.isNull()) {
      firstPoint = imagePos;
      firstPointItem = drawPoint(firstPoint);
    } else {
      QPointF secondPoint = imagePos;
      secondPointItem = drawPoint(secondPoint);

      QLineF line(firstPoint, secondPoint);
      lineItem = scene.addLine(line, QPen(Qt::red, 2));
      lineItem->setZValue(1);

      emit lineSelected(line);
      cancelGetLineFromUser();
    }
  } else if (selectingROI) {
    origin = event->pos();
    rubberBand->setGeometry(QRect(origin, QSize()));
    rubberBand->show();
  } else {
    // if not selecting a line handle clicks normally
    QGraphicsView::mousePressEvent(event);
  }
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event) {
  if (selectingROI)
    rubberBand->setGeometry(QRect(origin, event->pos()).normalized());
  QGraphicsView::mouseMoveEvent(event);
}

void ImageViewer::mouseReleaseEvent(QMouseEvent *event) {
  Q_UNUSED(event);
  if (selectingROI) {
    rubberBand->hide();
    QRect selected = rubberBand->geometry();
    QPointF topLeftQPoint = imageItem->mapFromScene(mapToScene(selected.topLeft()));
    QPointF bottomRightQPoint = imageItem->mapFromScene(mapToScene(selected.bottomRight()));
    QPen pen = QPen(Qt::red, 5);
    QBrush brush = QBrush(Qt::BrushStyle::NoBrush);

    roiItem = scene.addRect(QRectF(topLeftQPoint, bottomRightQPoint), pen, brush);
    roiItem->setZValue(1);

    cv::Point topLeft = cv::Point(topLeftQPoint.x(), topLeftQPoint.y());
    cv::Point bottomRight = cv::Point(bottomRightQPoint.x(), bottomRightQPoint.y());
    cv::Rect roi = cv::Rect(topLeft, bottomRight);

    emit roiSelected(roi);
  }
}

void ImageViewer::getLineFromUser() {
  cancelGetLineFromUser();
  setDragMode(QGraphicsView::NoDrag);
  selectingLine = true;
}

void ImageViewer::cancelGetLineFromUser() {
  selectingLine = false;
  firstPoint = QPointF();
  setDragMode(QGraphicsView::ScrollHandDrag);
}

void ImageViewer::deleteLine() {
  if (firstPointItem != nullptr) {
    scene.removeItem(firstPointItem);
    delete firstPointItem;
    firstPointItem = nullptr;
  }
  if (secondPointItem != nullptr) {
    scene.removeItem(secondPointItem);
    delete secondPointItem;
    secondPointItem = nullptr;
  }
  if (lineItem != nullptr) {
    scene.removeItem(lineItem);
    delete lineItem;
    lineItem = nullptr;
  }
}

void ImageViewer::getROIFromUser() {
  clearROI();
  cancelGetROIFromUser();
  setDragMode(QGraphicsView::NoDrag);
  selectingROI = true;
}

void ImageViewer::cancelGetROIFromUser() {
  selectingROI = false;
  origin = QPoint();
  setDragMode(QGraphicsView::ScrollHandDrag);
}

void ImageViewer::clearROI() {
  if (roiItem != nullptr) {
    scene.removeItem(roiItem);
    delete roiItem;
    roiItem = nullptr;
  }
}
