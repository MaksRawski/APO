#include "mdiChild.hpp"
#include "../imageProcessor.hpp"
#include "dialogs/DialogBuilder.hpp"
#include "dialogs/utils.hpp"
#include "imageLabel.hpp"
#include "parametersDialog.hpp"
#include <QFormLayout>
#include <QLineEdit>
#include <QPixmap>
#include <QScrollBar>
#include <QVBoxLayout>
#include <opencv2/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/opencv.hpp>
#include <qdialog.h>
#include <qdialogbuttonbox.h>
#include <qfiledialog.h>
#include <qscrollarea.h>
#include <qscrollbar.h>
#include <stdexcept>
#include <vector>

using imageProcessor::applyLUT;
using imageProcessor::LUT;

MdiChild::MdiChild(ImageWrapper imageWrapper) {
  tabWidget = new QTabWidget;
  tabWidget->setTabBarAutoHide(true);

  // all channels tab
  scrollArea = new QScrollArea;
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
  imageWrapper = image;
  QImage qimage = imageWrapper.generateQImage();
  if (qimage.isNull())
    throw new std::runtime_error("Failed to generate a QImage!");

  QPixmap pixmap = QPixmap::fromImage(qimage);
  imageLabel->setImage(pixmap);
  resize(pixmap.size() + CHILD_IMAGE_MARGIN);
  updateChannelNames();
  regenerateChannels();
  emitImageUpdatedSignal();
}

namespace {
struct ScrollbarsValues {
  int horizontal, vertical;
};
ScrollbarsValues getScrollbarsValues(const QScrollArea &scrollArea) {
  return ScrollbarsValues{scrollArea.horizontalScrollBar()->value(),
                          scrollArea.verticalScrollBar()->value()};
}
void setScrollbarsValues(QScrollArea &scrollArea, ScrollbarsValues values) {
  scrollArea.horizontalScrollBar()->setValue(values.horizontal);
  scrollArea.verticalScrollBar()->setValue(values.vertical);
}
} // namespace

// sets an image but maintains window size, image scale and scrollbars' positions
void MdiChild::swapImage(const ImageWrapper &image) {
  QSize size = this->size();
  double prevScale = getImageScale();
  auto oldScrollbarValues = getScrollbarsValues(getScrollArea(tabIndex));

  setImage(image);
  resize(size);
  setImageScale(prevScale);
  setScrollbarsValues(getScrollArea(tabIndex), oldScrollbarValues);
}

double MdiChild::getImageScale() const { return getImageLabel(tabIndex).getImageScale(); }
void MdiChild::setImageScale(double scale) { getImageLabel(tabIndex).setImageScale(scale); }

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

QScrollArea &MdiChild::getScrollArea(int index) const {
  switch (index) {
  case 0:
    return *scrollArea;
  case 1:
    return *scrollArea1;
  case 2:
    return *scrollArea2;
  case 3:
    return *scrollArea3;
  default:
    throw std::invalid_argument("Invalid tab index " + std::to_string(index));
  }
}

ImageLabel &MdiChild::getImageLabel(int index) const {
  switch (index) {
  case 0:
    return *imageLabel;
  case 1:
    return *imageLabel1;
  case 2:
    return *imageLabel2;
  case 3:
    return *imageLabel3;
  default:
    throw std::invalid_argument("Invalid tab index " + std::to_string(index));
  }
}

const ImageWrapper &MdiChild::getImageWrapper(int index) const {
  switch (index) {
  case 0:
    return imageWrapper;
  case 1:
    return imageWrapper1;
  case 2:
    return imageWrapper2;
  case 3:
    return imageWrapper3;
  default:
    throw std::invalid_argument("Invalid tab index " + std::to_string(index));
  }
}

void MdiChild::tabChanged(int newTabIndex) {
  // NOTE: this will happen only if there are no widgets in the tabWidget
  // which should be never
  if (newTabIndex < 0)
    return;

  // set the image scale to whatever the user had in the previous tab
  double prevScale = getImageLabel(tabIndex).getImageScale();
  getImageLabel(newTabIndex).setImageScale(prevScale);

  // set the scrollbars to the position they were in the previous tab
  auto prevScrollbarsValues = getScrollbarsValues(getScrollArea(tabIndex));
  setScrollbarsValues(getScrollArea(newTabIndex), prevScrollbarsValues);

  tabIndex = newTabIndex;
  emitImageUpdatedSignal();
}

void MdiChild::emitImageUpdatedSignal() const { emit imageUpdated(getImageWrapper(tabIndex)); }

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
  if (imageWrapper.getMat().channels() != 3)
    return;

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
  auto params = rangeStretchDialog(this);

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
  auto res = posterizeDialog(this);
  if (!res.has_value())
    return;
  LUT lut = imageProcessor::posterize(res.value());
  swapImage(applyLUT(imageWrapper, lut));
}

void MdiChild::blurMean() {
  auto res = kernelBorderDialog(this);

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
  auto res = kernelBorderDialog(this);
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
  auto [k, dir, borderType] = res.value();
  cv::Mat mat;
  if (dir == SobelDirections::Horizontal)
    cv::Sobel(imageWrapper.getMat(), mat, CV_8UC1, 1, 0, k, 1, 0, borderType);
  else
    cv::Sobel(imageWrapper.getMat(), mat, CV_8UC1, 0, 1, k, 1, 0, borderType);

  swapImage(mat);
}
void MdiChild::edgeDetectLaplacian() {
  auto res = kernelBorderDialog(this);
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
  auto res = choosableMaskDialog(this, LaplacianMasks::mats, LaplacianMasks::names);
  if (!res.has_value())
    return;

  auto [mask, borderType] = res.value();
  cv::Mat out;

  cv::filter2D(imageWrapper.getMat(), out, -1, mask, cv::Point(-1, -1), 0, borderType);
  swapImage(out);
}

void MdiChild::edgeDetectPrewitt() {
  auto res = choosableMaskDialog(this, PrewittMasks::mats, PrewittMasks::names);
  if (!res.has_value())
    return;

  auto [mask, borderType] = res.value();
  cv::Mat out;

  cv::filter2D(imageWrapper.getMat(), out, -1, mask, cv::Point(-1, -1), 0, borderType);
  swapImage(out);
}

void MdiChild::customMask() {}

void MdiChild::customTwoStageFilter() {
  auto res = Dialog(this, QString("Enter a mask"),
                    InputSpec<DialogValue::ComposableMask>{"Mask", KernelSizes::values,
                                                           LaplacianMasks::mats[0]}, BorderTypes::inputSpec)
                 .run();
  if (!res.has_value())
    return;
  auto [mask, borderType] = res.value();
  cv::Mat out;

  cv::filter2D(imageWrapper.getMat(), out, -1, mask, cv::Point(-1, -1), 0, borderType);
  swapImage(out);

}
