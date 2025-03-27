#include "imageLabel.hpp"
#include <qabstractscrollarea.h>
#include <qpixmap.h>
#include <qscrollarea.h>
#include <qscrollbar.h>
#include <qsharedpointer.h>

ImageLabel::ImageLabel(QWidget *parent) : QLabel(parent), scaleFactor(1.0) {
  setAlignment(Qt::AlignCenter);
}

void ImageLabel::wheelEvent(QWheelEvent *event) {
  double factor = (event->angleDelta().y() > 0) ? 1.1 : 0.9;
  double newScale = scaleFactor * factor;

  if (newScale < 0.1 || newScale > 10.0)
    return;

  setImageScale(newScale);

  // adjust scrollbars
  // HACK: for some reason the direct parent this widget is qt_scrollarea_viewport so must
  // take the parent of that, this widget should obviosuly not even rely on that, so
  // maybe using signals would be better
  QScrollArea* scrollArea = qobject_cast<QScrollArea*>(parentWidget()->parentWidget());
  if (scrollArea == nullptr) return;

  QPoint mousePos = event->position().toPoint();
  QScrollBar *hBar = scrollArea->horizontalScrollBar();
  QScrollBar *vBar = scrollArea->verticalScrollBar();

  hBar->setValue(hBar->value() + mousePos.x() * (factor - 1));
  vBar->setValue(vBar->value() + mousePos.y() * (factor - 1));
}

void ImageLabel::setImage(const QPixmap &pixmap) {
  originalPixmap = pixmap;
  setImageScale(1.0);
}
void ImageLabel::setImageScale(double scale) {
  scaleFactor = scale;
  if (!originalPixmap.isNull()) {
    Qt::TransformationMode mode =
        (scale > 2.0) ? Qt::FastTransformation : Qt::SmoothTransformation;
    setPixmap(originalPixmap.scaled(originalPixmap.size() * scale,
                                    Qt::KeepAspectRatio, mode));
  }
}

QPixmap ImageLabel::getImage() const { return originalPixmap; }

double ImageLabel::getImageScale() const { return scaleFactor; }
