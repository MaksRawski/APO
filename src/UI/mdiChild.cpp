#include "mdiChild.hpp"
#include <QPixmap>
#include <QVBoxLayout>
#include <qfileinfo.h>
#include <qnamespace.h>
#include <qpixmap.h>
#include <stdexcept>

MdiChild::MdiChild() {
  QScrollArea *scrollArea = new QScrollArea;
  scrollArea->setWidgetResizable(true);
  imageLabel = new ImageLabel(scrollArea);
  scrollArea->setWidget(imageLabel);

  setAttribute(Qt::WA_DeleteOnClose);
  setWidget(scrollArea);
}

void MdiChild::loadImage(QString filePath) {
  QString fileName = QFileInfo(filePath).fileName();
  imageWrapper = new ImageWrapper(filePath);

  QImage image = imageWrapper->generateQImage();
  if (image.isNull())
    throw new std::runtime_error("Failed to generate a QImage!");

  QPixmap pixmap = QPixmap::fromImage(image);
  imageLabel->setImage(pixmap);
  updateImageName(fileName);

  emit imageUpdated(*imageWrapper);
}
void MdiChild::setImageScale(double zoom) { imageLabel->setImageScale(zoom); }

void MdiChild::updateImageName(QString name) {
  imageName = name;
  auto imageFormat = pixelFormatToString(imageWrapper->getFormat());
  setWindowTitle(QString("[%1] %2").arg(imageFormat).arg(name));
}
