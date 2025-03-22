#include "mdiChild.hpp"
#include <QPixmap>

MdiChild::MdiChild(QPixmap pixmap) {
  imageLabel = new ImageLabel(this);
  imageLabel->setImage(pixmap);
  setWidget(imageLabel);
  setWidgetResizable(true);
}
