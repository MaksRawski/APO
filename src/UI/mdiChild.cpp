#include "mdiChild.hpp"
#include <QPixmap>
#include <qnamespace.h>

MdiChild::MdiChild() {
  imageLabel = new ImageLabel(this);
  setAttribute(Qt::WA_DeleteOnClose);
  setWidget(imageLabel);
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
void MdiChild::updatePixmap(QPixmap pixmap, QString pixmapName) {
  updatePixmap(pixmap);
  setWindowTitle(QString("[%1] %2").arg(toString(imageType)).arg(pixmapName));
}

QPixmap MdiChild::getPixmap() const{
  return imageLabel->getImage();
}
