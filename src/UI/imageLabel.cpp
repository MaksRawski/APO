#include "imageLabel.hpp"
#include <qpixmap.h>

ImageLabel::ImageLabel(QWidget *parent) : QLabel(parent), scaleFactor(1.0) {
  setAlignment(Qt::AlignCenter);
}

void ImageLabel::wheelEvent(QWheelEvent *event) {
  double factor = (event->angleDelta().y() > 0) ? 1.1 : 0.9;
  double newScale = scaleFactor * factor;

  // Prevent extreme zoom levels
  if (newScale < 0.1 || newScale > 10.0)
    return;

  scaleFactor = newScale;
  updateImageSize();
}

void ImageLabel::setImage(const QPixmap &pixmap) {
  originalPixmap = pixmap;
  scaleFactor = 1.0;
  updateImageSize();
}

QPixmap ImageLabel::getImage() const {
  return originalPixmap;
}

void ImageLabel::updateImageSize() {
  if (!originalPixmap.isNull()) {
    Qt::TransformationMode mode = (scaleFactor > 2.0)
                                      ? Qt::FastTransformation
                                      : Qt::SmoothTransformation;
    setPixmap(originalPixmap.scaled(originalPixmap.size() * scaleFactor,
                                    Qt::KeepAspectRatio, mode));
  }
}
