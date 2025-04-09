#include "mdiChild.hpp"
#include "imageLabel.hpp"
#include <QPixmap>
#include <QVBoxLayout>
#include <qfileinfo.h>
#include <qnamespace.h>
#include <qpixmap.h>
#include <qscrollarea.h>
#include <qtabwidget.h>
#include <stdexcept>

MdiChild::MdiChild() {
  tabWidget = new QTabWidget;

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
  ImageLabel *imageLabel1 = new ImageLabel(scrollArea1);
  scrollArea1->setWidget(imageLabel1);
  tabWidget->insertTab(1, scrollArea1, "Red");

  // 2nd channel, Green by default
  scrollArea2 = new QScrollArea;
  scrollArea2->setWidgetResizable(true);
  // stub imageLabel, will be properly set on the first change to any channel
  ImageLabel *imageLabel2 = new ImageLabel(scrollArea2);
  scrollArea2->setWidget(imageLabel2);
  tabWidget->insertTab(2, scrollArea2, "Green");

  // 3rd channel, Blue by default
  scrollArea3 = new QScrollArea;
  scrollArea3->setWidgetResizable(true);
  // stub imageLabel, will be properly set on the first change to any channel
  ImageLabel *imageLabel3 = new ImageLabel(scrollArea3);
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
  setImageName(fileName);
}

void MdiChild::setImage(QPixmap pixmap) {
  imageLabel->setImage(pixmap);
  resize(pixmap.size() + QSize(30, 70));
  setImageName(imageName);
  updateChannelNames();
  emit imageUpdated(*imageWrapper);
}

void MdiChild::setImageScale(double zoom) { imageLabel->setImageScale(zoom); }

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
}
void MdiChild::toLab() {
  *imageWrapper = imageWrapper->toLab();
  setImage(QPixmap::fromImage(imageWrapper->generateQImage()));
}
void MdiChild::toHSV() {
  *imageWrapper = imageWrapper->toHSV();
  setImage(QPixmap::fromImage(imageWrapper->generateQImage()));
}
void MdiChild::toRGB() {
  *imageWrapper = imageWrapper->toRGB();
  setImage(QPixmap::fromImage(imageWrapper->generateQImage()));
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

  // ignore the initial signal emission (not sure even if there is one)
  if (index == 0 && prevTabIndex == 0)
    return;

  if (index == 0) {
    // TODO: recreate the image from channels
  } else if (prevTabIndex == 0) {
    // split the image into channels
    std::vector<ImageWrapper> imageWrappers = imageWrapper->splitChannels();

    if (imageWrappers.size() < 3)
      throw new std::runtime_error("Failed to split channels!");
    imageWrapper1 = imageWrappers[0];
    imageWrapper2 = imageWrappers[1];
    imageWrapper3 = imageWrappers[2];

    ImageLabel *imageLabel1 = new ImageLabel;
    ImageLabel *imageLabel2 = new ImageLabel;
    ImageLabel *imageLabel3 = new ImageLabel;

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
  switch (index) {
  case 1:
    emit imageUpdated(imageWrapper1);
    break;
  case 2:
    emit imageUpdated(imageWrapper2);
    break;
  case 3:
    emit imageUpdated(imageWrapper3);
    break;
  }
  prevTabIndex = index;
}
