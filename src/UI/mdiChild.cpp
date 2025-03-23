#include "mdiChild.hpp"
#include <QPixmap>

MdiChild::MdiChild() {
  imageLabel = new ImageLabel(this);
  setWidget(imageLabel);
  setWidgetResizable(true);
}

void MdiChild::updatePixmap(QPixmap pixmap) {
  imageLabel->setImage(pixmap);
  emit pixmapUpdated(pixmap);
}
