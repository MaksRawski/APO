#include "mdiChild.hpp"
#include "../imageProcessor.hpp"
#include "imageLabel.hpp"
#include "parametersDialog.hpp"
#include <QFormLayout>
#include <QLineEdit>
#include <QPixmap>
#include <QVBoxLayout>
#include <opencv2/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/opencv.hpp>
#include <qdialog.h>
#include <qdialogbuttonbox.h>
#include <qfiledialog.h>

using imageProcessor::applyLUT;
using imageProcessor::LUT;

MdiChild::MdiChild(ImageWrapper imageWrapper) {
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
  setImage(imageWrapper);
}

void MdiChild::setImage(const ImageWrapper &image) {
  imageWrapper = ImageWrapper(image);
  QImage qimage = imageWrapper.generateQImage();
  if (qimage.isNull())
    throw new std::runtime_error("Failed to generate a QImage!");

  QPixmap pixmap = QPixmap::fromImage(qimage);
  imageLabel->setImage(pixmap);
  resize(pixmap.size() + CHILD_IMAGE_MARGIN);
  updateChannelNames();
  regenerateChannels();

  emit imageUpdated(imageWrapper);
}

// sets an image but maintains window size and image scale
void MdiChild::swapImage(const ImageWrapper &image) {
  QSize size = this->size();
  double prevZoom = getImageScale();

  setImage(image);
  resize(size);
  setImageScale(prevZoom);
}

void MdiChild::setImageScale(double scale) { imageLabel->setImageScale(scale); }

void MdiChild::updateChannelNames() {
  auto imageFormat = imageWrapper.getFormat();
  if (imageFormat == PixelFormat::Grayscale8) {
    tabWidget->removeTab(3);
    tabWidget->removeTab(2);
    tabWidget->removeTab(1);
  } else if (tabWidget->count() < 3) {
    auto names = PixelFormatUtils::channelNmaes(imageFormat);
    tabWidget->addTab(scrollArea1, QString::fromStdString(names[0]));
    tabWidget->addTab(scrollArea2, QString::fromStdString(names[1]));
    tabWidget->addTab(scrollArea3, QString::fromStdString(names[2]));
  }
}

void MdiChild::toGrayscale() {
  imageWrapper = imageWrapper.toGrayscale();
  swapImage(imageWrapper);
  setImageName(imageName); // update type in the window title
}
void MdiChild::toLab() {
  imageWrapper = imageWrapper.toLab();
  swapImage(imageWrapper);
  setImageName(imageName); // update type in the window title
}
void MdiChild::toHSV() {
  imageWrapper = imageWrapper.toHSV();
  swapImage(imageWrapper);
  setImageName(imageName); // update type in the window title
}
void MdiChild::toRGB() {
  imageWrapper = imageWrapper.toRGB();
  swapImage(imageWrapper);
  setImageName(imageName); // update type in the window title
}

void MdiChild::setImageName(QString name) {
  imageName = name;
  std::string imageFormat = PixelFormatUtils::toString(imageWrapper.getFormat());
  setWindowTitle(QString("[%1] %2").arg(QString::fromStdString(imageFormat), name));
}

void MdiChild::tabChanged(int newTabIndex) {
  // NOTE: this will happen only if there are no widgets in the tabWidget
  // which should be never
  if (newTabIndex < 0)
    return;

  // if switching to a channel from an image, regenerate all channels
  if (tabIndex == 0 && newTabIndex != 0) {
    regenerateChannels();
  }

  // set channel/image zoom to whatever the user had in the previous tab
  double prevZoom = 1.0;
  switch (tabIndex) {
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
  switch (newTabIndex) {
  case 0:
    imageLabel->setImageScale(prevZoom);
    break;

  case 1: {
    imageLabel1->setImageScale(prevZoom);
    break;
  }
  case 2: {
    imageLabel2->setImageScale(prevZoom);
    break;
  }
  case 3: {
    imageLabel3->setImageScale(prevZoom);
    break;
  }
  }

  tabIndex = newTabIndex;
  emitImageUpdatedSignal();
}

void MdiChild::emitImageUpdatedSignal() const {
  switch (tabIndex) {
  case 0: {
    emit imageUpdated(imageWrapper);
    break;
  }
  case 1: {
    emit imageUpdated(imageWrapper1);
    break;
  }
  case 2: {
    emit imageUpdated(imageWrapper2);
    break;
  }
  case 3: {
    emit imageUpdated(imageWrapper3);
    break;
  }
  }
}

QString MdiChild::getImageBasename() const {
  QFileInfo fileInfo(imageName);
  return fileInfo.completeBaseName();
}

QString MdiChild::getImageNameSuffix() const {
  QFileInfo fileInfo(imageName);
  return fileInfo.completeSuffix();
}

void MdiChild::negate() { swapImage(applyLUT(imageWrapper, imageProcessor::negate())); }

void MdiChild::regenerateChannels() {
  if (imageWrapper.getMat().channels() != 3) return;

  std::vector<ImageWrapper> imageWrappers = imageWrapper.splitChannels();

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

void MdiChild::normalize() { swapImage(imageProcessor::normalizeChannels(imageWrapper.getMat())); }

void MdiChild::equalize() { swapImage(imageProcessor::equalizeChannels(imageWrapper.getMat())); }

void MdiChild::rangeStretch() {
  auto params = rangeStretchDialog(this, 0, 255, 0, 255);
  if (!params.has_value())
    return;

  uchar p1, p2, q3, q4;
  std::tie(p1, p2, q3, q4) = params.value();

  swapImage(imageProcessor::rangeStretchChannels(imageWrapper.getMat(), p1, p2, q3, q4));
}

void MdiChild::save() {
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), imageName,
                                                  tr("Images (*.png *.jpg *.jpeg *.bmp)"));
  imageWrapper.generateQImage().save(fileName);
}

void MdiChild::rename() {
  QDialog dialog(this);
  dialog.setWindowTitle("Set image name");

  QFormLayout *formLayout = new QFormLayout(&dialog);
  QLineEdit *lineEdit = new QLineEdit();
  lineEdit->setText(imageName);
  formLayout->addRow(lineEdit);

  QDialogButtonBox *buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  formLayout->addRow(buttonBox);

  if (dialog.exec() == QDialog::Accepted) {
    setImageName(lineEdit->text());
  }
}

void MdiChild::posterize() {
  auto res = posterizeDialog(this, 2);
  if (!res.has_value())
    return;
  LUT lut = imageProcessor::posterize(res.value());
  swapImage(applyLUT(imageWrapper, lut));
}

void MdiChild::blurMean() {
  auto res = kernelSizeDialog(this);
  if (!res.has_value())
    return;
  uchar k;
  int borderType;
  std::tie(k, borderType) = res.value();

  cv::Mat out;
  cv::blur(imageWrapper.getMat(), out, cv::Size(k, k), cv::Point(-1, -1), borderType);
  swapImage(out);
}

void MdiChild::blurMedian() {
  auto res = kernelSizeDialog(this);
  if (!res.has_value())
    return;
  uchar k;
  int borderType;
  std::tie(k, borderType) = res.value();

  cv::Mat out;
  cv::medianBlur(imageWrapper.getMat(), out, k);
  swapImage(out);
}

void MdiChild::blurGaussian() {
  auto res = gaussianBlurDialog(this);
  if (!res.has_value())
    return;
  uchar k;
  double sigma;
  int borderType;
  std::tie(k, sigma, borderType) = res.value();

  cv::Mat mat;
  cv::GaussianBlur(imageWrapper.getMat(), mat, cv::Size(k, k), sigma, 0, borderType);
  swapImage(mat);
}

void MdiChild::edgeDetectSobel() {
  auto res = sobelDialog(this);
  if (!res.has_value())
    return;
  uchar k;
  Direction dir;
  int borderType;
  std::tie(k, dir, borderType) = res.value();
  cv::Mat mat;
  if (dir == Direction::Horizontal)
    cv::Sobel(imageWrapper.getMat(), mat, CV_8UC1, 1, 0, k, 1, 0, borderType);
  else
    cv::Sobel(imageWrapper.getMat(), mat, CV_8UC1, 0, 1, k, 1, 0, borderType);

  swapImage(mat);
}
void MdiChild::edgeDetectLaplacian() {
  auto res = kernelSizeDialog(this);
  if (!res.has_value())
    return;
  uchar k;
  int borderType;
  std::tie(k, borderType) = res.value();
  cv::Mat mat;
  cv::Laplacian(imageWrapper.getMat(), mat, CV_8UC1, k, 1, 0, borderType);
  swapImage(mat);
}

void MdiChild::edgeDetectCanny() {
  auto res = cannyDialog(this);
  if (!res.has_value())
    return;
  uchar k, start, end;
  int borderType;
  std::tie(k, start, end, borderType) = res.value();

  // manually padding
  int pad = k / 2;
  cv::Mat padded;
  cv::copyMakeBorder(imageWrapper.getMat(), padded, pad, pad, pad, pad, borderType);

  cv::Mat out;
  cv::Canny(padded, out, start, end, k);
  swapImage(out);
}

void MdiChild::sharpenLaplacian() {
  auto res = laplacianMaskDialog(this);
  if (!res.has_value())
    return;
  Mask3x3 mask;
  int borderType;
  std::tie(mask, borderType) = res.value();

  cv::Mat out;
  cv::Mat kernel(3, 3, CV_64F);
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      kernel.at<double>(i, j) = mask[i][j];

  cv::filter2D(imageWrapper.getMat(), out, -1, kernel, cv::Point(-1, -1), 0, borderType);
  swapImage(out);
}

void MdiChild::edgeDetectPrewitt() {
  auto res = prewittDirection(this);
  if (!res.has_value())
    return;
  Mask3x3 mask;
  int borderType;
  std::tie(mask, borderType) = res.value();

  cv::Mat out;
  cv::Mat kernel(3, 3, CV_64F);
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      kernel.at<double>(i, j) = mask[i][j];

  cv::filter2D(imageWrapper.getMat(), out, -1, kernel, cv::Point(-1, -1), 0, borderType);
  swapImage(out);
}
