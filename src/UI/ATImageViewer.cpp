#include "ATImageViewer.hpp"
#include "../imageProcessor.hpp"
#include "ImageViewer.hpp"
#include <opencv2/opencv.hpp>
#include <qevent.h>
#include <qgraphicsview.h>
#include <qnamespace.h>
#include <qpoint.h>

ATImageViewer::ATImageViewer(const ImageWrapper &image, QWidget *parent) : ImageViewer(parent) {
  setImage(image);
}

void ATImageViewer::setImage(const ImageWrapper &image) {
  // TODO: the image should take up the entire area
  QPixmap pixmap = image.generateQPixmap();
  ImageViewer::setImage(pixmap);
  this->ogImage = image;
  auto s = std::min(image.getWidth(), image.getHeight());
  pointRadius = s * POINT_RADIUS_PERCENT / 100.0;
  thickness = s * THICKNESS_PERCENT / 100.0;
  adjustSize();
}

ImageWrapper ATImageViewer::getTransformedImage() { return transformedImage; }

// returns index of a point in points if there was a point within tolerable manhatan length from the
// pos otherwise returns -1 expects position to be in image coordinates
int findNearestPoint(std::vector<QPointF> points, QPointF pos, float maxDistance) {
  int pointIndex = -1;
  for (uint i = 0; i < points.size(); ++i) {
    QPointF point = points[i];
    if (std::abs(point.x() - pos.x()) + std::abs(point.y() - pos.y()) < maxDistance) {
      pointIndex = i;
      break;
    }
  }
  return pointIndex;
}

void ATImageViewer::mousePressEvent(QMouseEvent *event) {
  // if left button is being held and a mouse is over a point then the point is being moved
  if (event->button() == Qt::LeftButton) {
    auto pos = getPosInImage(event);
    movingPoint = findNearestPoint(pointsPos, pos, pointRadius);

    // prevent default behaviour when point is being moved
    if (movingPoint > -1)
      return;
  }
  QGraphicsView::mousePressEvent(event);
}

void ATImageViewer::mouseMoveEvent(QMouseEvent *event) {
  if (movingPoint > -1) {
    // update movingPoint position
    auto imagePos = getPosInImage(event);
    pointsPos[movingPoint] = imagePos;

    auto ellipse = ellipsesPos[movingPoint];
    ellipse->setPos(imageItem->mapToScene(imagePos));
    lines[movingPoint]->setLine(QLineF(pointsOrigin[movingPoint], pointsPos[movingPoint]));
  } else {
    // if no point is selected, move the canvas
    QGraphicsView::mouseMoveEvent(event);
  }
}

void ATImageViewer::affineTransform() {
  std::vector<cv::Point2f> srcPoints;
  for (auto point : pointsOrigin)
    srcPoints.push_back(cv::Point2f(point.x(), point.y()));

  std::vector<cv::Point2f> dstPoints;
  for (auto point : pointsPos)
    dstPoints.push_back(cv::Point2f(point.x(), point.y()));

  cv::Mat dst = imageProcessor::affineTransform(ogImage.getMat(), srcPoints, dstPoints);
  transformedImage = ImageWrapper(dst);
  imageItem->setPixmap(transformedImage.generateQPixmap());
}

void ATImageViewer::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    if (movingPoint > -1) {
      if (pointsOrigin.size() == 3) {
        affineTransform();
      }
      movingPoint = -1;
    }
  } else if (event->button() == Qt::RightButton && pointsPos.size() < 3) {
    // add a point
    auto pos = getPosInImage(event);
    pointsOrigin.push_back(pos);
    pointsPos.push_back(pos);

    // add ellipses
    QPen pointSrcPen = POINT_SRC_PEN;
    pointSrcPen.setWidthF(thickness);
    QPen pointDstPen = POINT_DST_PEN;
    pointDstPen.setWidthF(thickness);

    auto *ellipseSrc = drawPoint(QPoint(0, 0), pointRadius, pointSrcPen, POINT_SRC_BRUSH);
    ellipseSrc->setPos(ellipseSrc->mapFromItem(imageItem, pos));
    ellipsesOrigin.push_back(ellipseSrc);

    // ellipse dst is going to be the "movable" point
    auto *ellipseDst = drawPoint(QPoint(0, 0), pointRadius, pointDstPen, POINT_DST_BRUSH);
    ellipseDst->setPos(ellipseDst->mapFromItem(imageItem, pos));
    ellipsesPos.push_back(ellipseDst);

    // add a line between origin of a point and its current location
    QPen linePen = LINE_PEN;
    linePen.setWidthF(thickness);
    auto *line = scene.addLine(QLineF(0, 0, 0, 0), linePen);
    line->setZValue(1);
    lines.push_back(line);

    movingPoint = -1;
    return;
  } else if (event->button() == Qt::MiddleButton) {
    // remove a point at pos
    int i = findNearestPoint(pointsPos, getPosInImage(event), pointRadius);
    if (i == -1) {
      i = findNearestPoint(pointsOrigin, getPosInImage(event), pointRadius);
    }
    if (i > -1) {
      pointsOrigin.erase(pointsOrigin.begin() + i);
      pointsPos.erase(pointsPos.begin() + i);

      scene.removeItem(ellipsesOrigin[i]);
      ellipsesOrigin.erase(ellipsesOrigin.begin() + i);

      scene.removeItem(ellipsesPos[i]);
      ellipsesPos.erase(ellipsesPos.begin() + i);

      scene.removeItem(lines[i]);
      lines.erase(lines.begin() + i);

      // reset transformation
      imageItem->setPixmap(ogImage.generateQPixmap());
    }
    movingPoint = -1;
    return;
  }

  QGraphicsView::mouseReleaseEvent(event);
}
