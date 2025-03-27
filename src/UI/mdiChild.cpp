#include "mdiChild.hpp"
#include <QPixmap>
#include <qnamespace.h>
#include <QVBoxLayout>

MdiChild::MdiChild() {
  QScrollArea *scrollArea = new QScrollArea;
  scrollArea->setWidgetResizable(true);
  imageLabel = new ImageLabel(scrollArea);
  scrollArea->setWidget(imageLabel);

  setAttribute(Qt::WA_DeleteOnClose);
  setWidget(scrollArea);
}

void MdiChild::updatePixmap(QPixmap pixmap) {
  imageLabel->setImage(pixmap);
  QImage im = pixmap.toImage();
  if (im.allGray()) {
    imageType = ImageType::GrayScale;
  } else {
    imageType = ImageType::RGB;
  }

  emit pixmapUpdated(pixmap);
}

void MdiChild::setImageScale(double zoom) { imageLabel->setImageScale(zoom); }

void MdiChild::updatePixmap(QPixmap pixmap, QString pixmapName) {
  updatePixmap(pixmap);
  setWindowTitle(QString("[%1] %2").arg(toString(imageType)).arg(pixmapName));
}

QPixmap MdiChild::getPixmap() const { return imageLabel->getImage(); }
