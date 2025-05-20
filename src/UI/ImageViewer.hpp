#pragma once

#include <QGraphicsPixmapItem>
#include <QGraphicsView>
#include <QLabel>
#include <QPixmap>
#include <QWheelEvent>
#include <opencv2/core/types.hpp>
#include <qgraphicsitem.h>
#include <qpoint.h>
#include <qrubberband.h>
#include <qscrollarea.h>
#include <qtmetamacros.h>
#include <qwidget.h>

class ImageViewer : public QGraphicsView {
  Q_OBJECT
public:
  explicit ImageViewer(QWidget *parent = nullptr);
  void setImage(const QPixmap &pixmap);
  void useImageTransform(const ImageViewer &other);
  void fit();
  void clear();

  // will allow lineSelected signal to be emitted once
  void getLineFromUser();
  // will cancel any pending line selections
  void cancelGetLineFromUser();
  void deleteLine();

  void getROIFromUser();
  void cancelGetROIFromUser();
  void clearROI();

  QPixmap getImage() const;

private:
  QGraphicsEllipseItem *drawPoint(QPointF point);

protected:
  void wheelEvent(QWheelEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
signals:
  void lineSelected(QLineF line);
  void roiSelected(cv::Rect roi);

private:
  QGraphicsScene scene;
  QGraphicsPixmapItem *imageItem = nullptr;
  const qreal zoomFactor;

  // line selection
  QPointF firstPoint;
  QGraphicsLineItem *lineItem = nullptr;
  QGraphicsEllipseItem *firstPointItem = nullptr;
  QGraphicsEllipseItem *secondPointItem = nullptr;
  bool selectingLine = false;

  // ROI selection
  bool selectingROI = false;
  QRubberBand *rubberBand = nullptr;
  QGraphicsRectItem *roiItem = nullptr;
  QPoint origin;
};
