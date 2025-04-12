#include "../imageProcessor.hpp"
#include "mdiChild.hpp"
#include "imageLabel.hpp"
#include <QPixmap>
#include <QVBoxLayout>
#include <opencv2/core.hpp>
#include <qfileinfo.h>
#include <qnamespace.h>
#include <qpixmap.h>
#include <qscrollarea.h>
#include <qtabwidget.h>
#include <stdexcept>

using imageProcessor::applyLUT;
using imageProcessor::LUT;

MdiChild::MdiChild() {
  tabWidget = new QTabWidget;
  tabWidget->setTabBarAutoHide(true);

  // all channels tab
  QScrollArea *scrollArea = new QScrollArea;
  scrollArea->setWidgetResizable(true);
  imageLabel = new ImageLabel(scrollArea);
  scrollArea->setWidget(imageLabel);
  tabWidget->insertTab(0, scrollArea, "All");

  // 1st channel, Red by default
  scrollArea1 = new QScrollArea;
  scrollArea1->setWidgetResizable(true);
  // stub imageLabel, will be properly set on the first change to any channel
  imageLabel1 = new ImageLabel(scrollArea1);
  scrollArea1->setWidget(imageLabel1);
  tabWidget->insertTab(1, scrollArea1, "Red");

  // 2nd channel, Green by default
  scrollArea2 = new QScrollArea;
  scrollArea2->setWidgetResizable(true);
  // stub imageLabel, will be properly set on the first change to any channel
  imageLabel2 = new ImageLabel(scrollArea2);
  scrollArea2->setWidget(imageLabel2);
  tabWidget->insertTab(2, scrollArea2, "Green");

  // 3rd channel, Blue by default
  scrollArea3 = new QScrollArea;
  scrollArea3->setWidgetResizable(true);
  // stub imageLabel, will be properly set on the first change to any channel
  imageLabel3 = new ImageLabel(scrollArea3);
  scrollArea1->setWidget(imageLabel3);
  tabWidget->insertTab(3, scrollArea3, "Blue");

  connect(tabWidget, &QTabWidget::currentChanged, this, &MdiChild::tabChanged);

  setAttribute(Qt::WA_DeleteOnClose);
  setWidget(tabWidget);
}

void MdiChild::loadImage(QString filePath) {
  QString fileName = QFileInfo(filePath).fileName();
  imageWrapper = new ImageWrapper(filePath);

  QImage image = imageWrapper->generateQImage();
  if (image.isNull())
    throw new std::runtime_error("Failed to generate a QImage!");

  QPixmap pixmap = QPixmap::fromImage(image);
  setImage(pixmap);
  resize(pixmap.size() + CHILD_IMAGE_MARGIN);
  setImageName(fileName);
}

void MdiChild::setImage(const ImageWrapper &image) {
  imageWrapper = new ImageWrapper(image);
  QImage qimage = imageWrapper->generateQImage();
  if (qimage.isNull())
    throw new std::runtime_error("Failed to generate a QImage!");

  QPixmap pixmap = QPixmap::fromImage(qimage);
  setImage(pixmap);
  resize(pixmap.size() + CHILD_IMAGE_MARGIN);
}

void MdiChild::setImage(const QPixmap &pixmap) {
  imageLabel->setImage(pixmap);
  updateChannelNames();

  // if the user is on the main image tab we don't need to regenerate channels
  // that will be done once the tab is switched
  if (prevTabIndex != 0) regenerateChannels();

  emit imageUpdated(*imageWrapper);
}

void MdiChild::setImageScale(double zoom) {
  imageLabel->setImageScale(zoom);
  this->zoom = zoom;
}

void MdiChild::updateChannelNames() {
  auto imageFormat = imageWrapper->getFormat();
  if (imageFormat == PixelFormat::Grayscale8) {
    tabWidget->removeTab(3);
    tabWidget->removeTab(2);
    tabWidget->removeTab(1);
  } else if (tabWidget->count() < 3) {
    tabWidget->addTab(scrollArea1, "R");
    tabWidget->addTab(scrollArea2, "G");
    tabWidget->addTab(scrollArea3, "B");
  }

  if (imageFormat == PixelFormat::HSV24) {
    tabWidget->setTabText(1, "Hue");
    tabWidget->setTabText(2, "Saturation");
    tabWidget->setTabText(3, "Value");
  } else if (imageFormat == PixelFormat::Lab24) {
    tabWidget->setTabText(1, "L");
    tabWidget->setTabText(2, "a");
    tabWidget->setTabText(3, "b");
  } else if (imageFormat == PixelFormat::BGR24) {
    tabWidget->setTabText(1, "Red");
    tabWidget->setTabText(2, "Green");
    tabWidget->setTabText(3, "Blue");
  }
}

void MdiChild::toGrayscale() {
  *imageWrapper = imageWrapper->toGrayscale();
  setImage(QPixmap::fromImage(imageWrapper->generateQImage()));
  setImageName(imageName); // update type in the window title
}
void MdiChild::toLab() {
  *imageWrapper = imageWrapper->toLab();
  setImage(QPixmap::fromImage(imageWrapper->generateQImage()));
  setImageName(imageName); // update type in the window title
}
void MdiChild::toHSV() {
  *imageWrapper = imageWrapper->toHSV();
  setImage(QPixmap::fromImage(imageWrapper->generateQImage()));
  setImageName(imageName); // update type in the window title
}
void MdiChild::toRGB() {
  *imageWrapper = imageWrapper->toRGB();
  setImage(QPixmap::fromImage(imageWrapper->generateQImage()));
  setImageName(imageName); // update type in the window title
}

void MdiChild::setImageName(QString name) {
  imageName = name;
  auto imageFormat = pixelFormatToString(imageWrapper->getFormat());
  setWindowTitle(QString("[%1] %2").arg(imageFormat).arg(name));
}

void MdiChild::tabChanged(int index) {
  // NOTE: this will happen only if there are no widgets in the tabWidget
  // which should be never
  if (index < 0)
    return;

  // if switching to a channel from an image, regenerate all channels
  if (prevTabIndex == 0 && index != 0) {
    regenerateChannels();
  }

  // set channel/image zoom to whatever the user had in the previous tab
  double prevZoom;
  switch (prevTabIndex) {
  case 0:
    prevZoom = imageLabel->getImageScale();
    break;
  case 1:
    prevZoom = imageLabel1->getImageScale();
    break;
  case 2:
    prevZoom = imageLabel2->getImageScale();
    break;
  case 3:
    prevZoom = imageLabel3->getImageScale();
    break;
  }
  switch (index) {
  case 0: {
    emit imageUpdated(*imageWrapper);
    imageLabel->setImageScale(prevZoom);
    break;
  }
  case 1: {
    emit imageUpdated(imageWrapper1);
    imageLabel1->setImageScale(prevZoom);
    break;
  }
  case 2: {
    emit imageUpdated(imageWrapper2);
    imageLabel2->setImageScale(prevZoom);
    break;
  }
  case 3: {
    emit imageUpdated(imageWrapper3);
    imageLabel3->setImageScale(prevZoom);
    break;
  }
  }

  prevTabIndex = index;
}

QString MdiChild::getImageBasename() const {
  QFileInfo fileInfo(imageName);
  return fileInfo.completeBaseName();
}

QString MdiChild::getImageNameSuffix() const {
  QFileInfo fileInfo(imageName);
  return fileInfo.completeSuffix();
}

void MdiChild::negate() {
  LUT neg = imageProcessor::negate();
  ImageWrapper res = applyLUT(*imageWrapper, neg);
  setImage(res);
}

void MdiChild::regenerateChannels() {
  std::vector<ImageWrapper> imageWrappers = imageWrapper->splitChannels();

  if (imageWrappers.size() < 3)
    throw new std::runtime_error("Failed to split channels!");

  imageWrapper1 = imageWrappers[0];
  imageWrapper2 = imageWrappers[1];
  imageWrapper3 = imageWrappers[2];

  imageLabel1 = new ImageLabel;
  imageLabel2 = new ImageLabel;
  imageLabel3 = new ImageLabel;

  QPixmap pixmap1 = QPixmap::fromImage(imageWrapper1.generateQImage());
  QPixmap pixmap2 = QPixmap::fromImage(imageWrapper2.generateQImage());
  QPixmap pixmap3 = QPixmap::fromImage(imageWrapper3.generateQImage());

  imageLabel1->setImage(pixmap1);
  imageLabel2->setImage(pixmap2);
  imageLabel3->setImage(pixmap3);
  scrollArea1->setWidget(imageLabel1);
  scrollArea2->setWidget(imageLabel2);
  scrollArea3->setWidget(imageLabel3);
}

void MdiChild::normalize() {
  double min, max;
  cv::minMaxLoc(imageWrapper->getMat(), &min, &max);
  LUT stretched = imageProcessor::stretch((int)min, (int)max, 0, 255);
  setImage(applyLUT(*imageWrapper, stretched));
}

void MdiChild::equalize() {
  setImage(imageProcessor::equalizeChannels(imageWrapper->getMat()));
}
