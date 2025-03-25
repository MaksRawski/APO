#include "imageLabel.hpp"
#include <qpixmap.h>

ImageLabel::ImageLabel(QWidget *parent) : QLabel(parent), scaleFactor(1.0) {
  setAlignment(Qt::AlignCenter);
}

void ImageLabel::wheelEvent(QWheelEvent *event) {
  double factor = (event->angleDelta().y() > 0) ? 1.1 : 0.9;
  double newScale = scaleFactor * factor;

  if (newScale < 0.1 || newScale > 10.0)
    return;

  setImageScale(newScale);
}

void ImageLabel::setImage(const QPixmap &pixmap) {
  originalPixmap = pixmap;
  setImageScale(1.0);
}
void ImageLabel::setImageScale(double scale) {
  scaleFactor = scale;
  if (!originalPixmap.isNull()) {
    Qt::TransformationMode mode = (scale > 2.0)
                                      ? Qt::FastTransformation
                                      : Qt::SmoothTransformation;
    setPixmap(originalPixmap.scaled(originalPixmap.size() * scale,
                                    Qt::KeepAspectRatio, mode));
  }
}

QPixmap ImageLabel::getImage() const {
  return originalPixmap;
}

double ImageLabel::getImageScale() const {
  return scaleFactor;
}
