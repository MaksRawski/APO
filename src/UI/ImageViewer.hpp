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

  // will allow lineSelected signal to be emitted after two points are selected
  void getLineFromUser();
  // will cancel any pending line selections
  void cancelGetLineFromUser();
  void deleteLine();

  // will allow roiSelected signal to be emitted once a rectangle has been selected
  void getROIFromUser();
  void cancelGetROIFromUser();
  void clearROI();

  QPixmap getImage() const;

protected:
  void wheelEvent(QWheelEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

  QGraphicsEllipseItem *drawPoint(QPointF point, const double radius, const QPen &pen,
                                  const QBrush &brush);
  // get position of a mouse relative to the image
  QPointF getPosInImage(QMouseEvent *event);

  QGraphicsScene scene;
  QGraphicsPixmapItem *imageItem = nullptr;

signals:
  void lineSelected(QLineF line);
  void roiSelected(cv::Rect roi);

private:
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
