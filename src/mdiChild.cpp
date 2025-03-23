#include "mdiChild.hpp"
#include <QPixmap>

MdiChild::MdiChild(QPixmap pixmap) {
  imageLabel = new ImageLabel(this);
  imageLabel->setImage(pixmap);
  setWidget(imageLabel);
  setWidgetResizable(true);

  // emit the signal after the initial setup
  emit pixmapUpdated(pixmap);
  qDebug() << "mdiChild: updated pixmap";
}

void MdiChild::updatePixmap(QPixmap pixmap) {
  imageLabel->setImage(pixmap);
  emit pixmapUpdated(pixmap);
  qDebug() << "mdiChild: updated pixmap";
}
