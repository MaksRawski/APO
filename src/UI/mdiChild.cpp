#include "mdiChild.hpp"
#include "imageLabel.hpp"
#include <QPixmap>
#include <QVBoxLayout>
#include <exception>
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

  // // 1st channel, Red by default
  // QScrollArea *scrollArea1 = new QScrollArea;
  // scrollArea1->setWidgetResizable(true);
  // imageLabel1 = new ImageLabel(scrollArea1);
  // scrollArea1->setWidget(imageLabel1);
  // tabWidget->insertTab(1, scrollArea1, "Red");

  // // 2nd channel, Green by default
  // QScrollArea *scrollArea2 = new QScrollArea;
  // scrollArea2->setWidgetResizable(true);
  // imageLabel2 = new ImageLabel(scrollArea2);
  // scrollArea2->setWidget(imageLabel2);
  // tabWidget->insertTab(2, scrollArea2, "Green");

  // // 3rd channel, Blue by default
  // QScrollArea *scrollArea3 = new QScrollArea;
  // scrollArea3->setWidgetResizable(true);
  // imageLabel3 = new ImageLabel(scrollArea3);
  // scrollArea3->setWidget(imageLabel3);
  // tabWidget->insertTab(3, scrollArea3, "Blue");

  // connect(tabWidget, &QTabWidget::currentChanged, this, &MdiChild::tabChanged);

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
  qDebug() << "setImage!";
  imageLabel->setImage(pixmap);
  resize(pixmap.size() + QSize(30, 70));
  setImageName(imageName);
  emit imageUpdated(*imageWrapper);
}

void MdiChild::setImageScale(double zoom) { imageLabel->setImageScale(zoom); }

void MdiChild::updateChannelNames() {
  auto imageFormat = imageWrapper->getFormat();
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
  ImageWrapper im = imageWrapper->toGrayscale();
  setImage(QPixmap::fromImage(im.generateQImage()));
}
void MdiChild::toLab() {
  ImageWrapper im = imageWrapper->toLab();
  setImage(QPixmap::fromImage(im.generateQImage()));
}
void MdiChild::toHSV() {
  ImageWrapper im = imageWrapper->toHSV();
  setImage(QPixmap::fromImage(im.generateQImage()));
}
void MdiChild::toRGB() {
  ImageWrapper im = imageWrapper->toGrayscale();
  setImage(QPixmap::fromImage(im.generateQImage()));
}

void MdiChild::setImageName(QString name) {
  imageName = name;
  auto imageFormat = pixelFormatToString(imageWrapper->getFormat());
  setWindowTitle(QString("[%1] %2").arg(imageFormat).arg(name));
}

void MdiChild::tabChanged(int index) {
  // NOTE: this will happen only if there are no widgets in the tabWidget
  // which should be never
  if (index < 0) return;

  // ignore the initial signal emission (not sure even if there is one)
  if (index == 0 && prevTabIndex == 0) return;

  if (index == 0) {
    // TODO: recreate the image from channels
  } else if (prevTabIndex == 0) {
    // split the image into channels
    std::vector<ImageWrapper> imageWrappers = imageWrapper->splitChannels();

    if (imageWrappers.size() < 3) throw new std::runtime_error("Failed to split channels!");
    imageWrapper1 = imageWrappers[0];
    imageWrapper2 = imageWrappers[1];
    imageWrapper3 = imageWrappers[2];

    imageLabel1 = new ImageLabel;
    imageLabel2 = new ImageLabel;
    imageLabel3 = new ImageLabel;

    QImage image1 = imageWrapper1.generateQImage();
    QImage image2 = imageWrapper2.generateQImage();
    QImage image3 = imageWrapper3.generateQImage();
    QPixmap pixmap1 = QPixmap::fromImage(image1);
    QPixmap pixmap2 = QPixmap::fromImage(image2);
    QPixmap pixmap3 = QPixmap::fromImage(image3);
    // qDebug() << "image1:" << image1.size() << "pixmap1:" << pixmap1.size();
    // qDebug() << "image2:" << image2.size() << "pixmap2:" << pixmap2.size();
    // qDebug() << "image3:" << image3.size() << "pixmap3:" << pixmap3.size();

    imageLabel1->setImage(pixmap1);
    imageLabel2->setImage(pixmap2);
    imageLabel3->setImage(pixmap3);
  }
  prevTabIndex = index;
}
