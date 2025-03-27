#include "mdiChild.hpp"
#include "../imageProcessor.hpp"
#include <QPixmap>
#include <QVBoxLayout>
#include <qnamespace.h>
#include <qpixmap.h>

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

void MdiChild::updateImageName(QString name) {
  imageName = name;
  setWindowTitle(QString("[%1] %2").arg(toString(imageType)).arg(name));
}

QPixmap MdiChild::getPixmap() const { return imageLabel->getImage(); }

void MdiChild::toGrayscale() {
  imageType = ImageType::GrayScale;

  const QImage image = getPixmap().toImage();
  updatePixmap(QPixmap::fromImage(imageProcessor::toGrayScale(image)));
  // update the image type in the window title
  updateImageName(imageName);
}
