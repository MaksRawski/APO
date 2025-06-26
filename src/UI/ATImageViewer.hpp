#pragma once
#include "../imageWrapper.hpp"
#include "ImageViewer.hpp"
#include <opencv2/core.hpp>
#include <qcolor.h>
#include <qevent.h>
#include <qgraphicsitem.h>
#include <qwidget.h>
#include <vector>

// radius of a point as a percent of the entire image
const int POINT_RADIUS_PERCENT = 5;

// colors of the original position of a point
const QBrush POINT_SRC_BRUSH = QBrush(QColorConstants::Transparent);
const QPen POINT_SRC_PEN = QPen(QColor(200, 100, 100, 255), 1);

// colors of the moved position of a point
const QBrush POINT_DST_BRUSH = QBrush(QColorConstants::Transparent);
const QPen POINT_DST_PEN = QPen(QColor(100, 100, 200, 255), 1);

// line between points color
const QPen LINE_PEN = QPen(QColor(150, 100, 200, 255), 5);

// Affine Transformation Image Viewer
// wrapper around ImageViewer that allows affine transformation to be done
// on the inner image by the user via first selecting points and then moving them
class ATImageViewer : public ImageViewer {
  Q_OBJECT
public:
  explicit ATImageViewer(const ImageWrapper &image, QWidget *parent = nullptr);
  void setImage(const ImageWrapper &image);
  ImageWrapper getImageWrapper();

protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  void affineTransform();
  ImageWrapper image;
  // real value of a radius of a point as a POINT_RADIUS_PERCENT with respect to the current image
  float pointRadius;
  // -1 means no point is being moved, otherwise indicates index of a point being moved
  int movingPoint = -1;
  // positions of points when they were added
  std::vector<QPointF> pointsOrigin;
  // positions of points after they were moved
  std::vector<QPointF> pointsPos;
  std::vector<QGraphicsEllipseItem *> ellipsesOrigin;
  std::vector<QGraphicsEllipseItem *> ellipsesPos;
  // lines between points
  std::vector<QGraphicsLineItem *> lines;

signals:
  // emitted 0.2s after last movement
  void pointsMoved();
};
