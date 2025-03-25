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
  emit pixmapUpdated(pixmap);
}

QPixmap MdiChild::getPixmap() const{
  return imageLabel->getImage();
}
