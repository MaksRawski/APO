#include "mdiChild.hpp"
#include "../imageProcessor.hpp"
#include "ImageViewer.hpp"
#include "dialogs/utils.hpp"
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
  mainImage = new ImageViewer;
  tabWidget->insertTab(0, mainImage, "All");

  // 1st channel, Red by default
  image1 = new ImageViewer;
  tabWidget->insertTab(1, image1, "Red");

  // 2nd channel, Green by default
  image2 = new ImageViewer;
  tabWidget->insertTab(2, image2, "Green");

  // 3rd channel, Blue by default
  image3 = new ImageViewer;
  tabWidget->insertTab(3, image3, "Blue");

  connect(tabWidget, &QTabWidget::currentChanged, this, &MdiChild::tabChanged);

  setAttribute(Qt::WA_DeleteOnClose);
  setWidget(tabWidget);
  setImage(imageWrapper);
}

void MdiChild::setImage(const ImageWrapper &image) {
  swapImage(image);
  mainImage->fit();
}

void MdiChild::swapImage(const ImageWrapper &image) {
  imageWrapper = image;
  QImage qimage = imageWrapper.generateQImage();
  if (qimage.isNull())
    throw new std::runtime_error("Failed to generate a QImage!");

  QPixmap pixmap = QPixmap::fromImage(qimage);
  mainImage->setImage(pixmap);
  updateChannelNames();
  regenerateChannels();
  emitImageUpdatedSignal();
}

void MdiChild::updateChannelNames() {
  auto imageFormat = imageWrapper.getFormat();
  if (imageFormat == PixelFormat::Grayscale8) {
    tabWidget->removeTab(3);
    tabWidget->removeTab(2);
    tabWidget->removeTab(1);
  } else if (tabWidget->count() < 3) {
    auto names = PixelFormatUtils::channelNmaes(imageFormat);
    tabWidget->addTab(image1, QString::fromStdString(names[0]));
    tabWidget->addTab(image2, QString::fromStdString(names[1]));
    tabWidget->addTab(image3, QString::fromStdString(names[2]));
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

void MdiChild::fitImage() {
  mainImage->fit();
  image1->fit();
  image2->fit();
  image3->fit();
}

ImageViewer &MdiChild::getImageViewer(int index) const {
  switch (index) {
  case 0:
    return *mainImage;
  case 1:
    return *image1;
  case 2:
    return *image2;
  case 3:
    return *image3;
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

  // set the image's zoom and pan to whatever the user had in the previous tab
  getImageViewer(newTabIndex).useImageTransform(getImageViewer(tabIndex));

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

  QPixmap pixmap1 = QPixmap::fromImage(imageWrapper1.generateQImage());
  QPixmap pixmap2 = QPixmap::fromImage(imageWrapper2.generateQImage());
  QPixmap pixmap3 = QPixmap::fromImage(imageWrapper3.generateQImage());

  image1->setImage(pixmap1);
  image2->setImage(pixmap2);
  image3->setImage(pixmap3);
}

void MdiChild::normalize() { swapImage(imageProcessor::normalizeChannels(imageWrapper.getMat())); }

void MdiChild::equalize() { swapImage(imageProcessor::equalizeChannels(imageWrapper.getMat())); }

void MdiChild::rangeStretch() {
  cv::Mat mat = imageWrapper.getMat();
  auto previewFn = [mat](uchar p1, uchar p2, uchar q3, uchar q4) {
    return imageProcessor::rangeStretchChannels(mat, p1, p2, q3, q4);
  };

  auto im = Dialog(this, QString("Enter parameters"), //
                   InputSpec<IntParam>{"p1", {0, 255}, 0}, InputSpec<IntParam>{"p2", {0, 255}, 255},
                   InputSpec<IntParam>{"q3", {0, 255}, 0}, InputSpec<IntParam>{"q4", {0, 255}, 255})
                .runWithPreview(previewFn);

  if (!im.has_value())
    return;

  swapImage(im.value());
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

void MdiChild::applyDialogFilter(std::optional<std::tuple<cv::Mat, int>> res) {
  if (!res.has_value())
    return;

  auto [kernel, borderType] = res.value();
  cv::Mat out;

  kernel /= cv::sum(kernel)[0];

  cv::filter2D(imageWrapper.getMat(), out, -1, kernel, cv::Point(-1, -1), 0, borderType);
  swapImage(out);
}

void MdiChild::sharpenLaplacian() {
  auto res = choosableMaskDialog(this, LaplacianMasks::mats, LaplacianMasks::names);
  applyDialogFilter(res);
}

void MdiChild::edgeDetectPrewitt() {
  auto res = choosableMaskDialog(this, PrewittMasks::mats, PrewittMasks::names);
  applyDialogFilter(res);
}

void MdiChild::customMask() {
  auto res = choosableMaskDialog(this, {BoxKernel::mat3}, {});
  applyDialogFilter(res);
}

void MdiChild::customTwoStageFilter() {
  auto res = Dialog(this, QString("Convolve a mask"),
                    InputSpec<DialogParam<DialogValue::ComposableMask, void>>{
                        "Mask", KernelSizes::values, BoxKernel::mat3},
                    BorderTypes::inputSpec)
                 .run();

  applyDialogFilter(res);
}

void MdiChild::morphologyErode() {
  auto res = structuringElementDialog(this);
  if (!res.has_value())
    return;

  auto [kernel, borderType] = res.value();

  cv::Mat out;
  cv::erode(imageWrapper.getMat(), out, kernel, cv::Point(-1, -1), 1, borderType);
  swapImage(out);
}

void MdiChild::morphologyDilate() {
  auto res = structuringElementDialog(this);
  if (!res.has_value())
    return;

  auto [kernel, borderType] = res.value();

  cv::Mat out;
  cv::dilate(imageWrapper.getMat(), out, kernel, cv::Point(-1, -1), 1, borderType);
  swapImage(out);
}

void MdiChild::morphologyOpen() {
  auto res = structuringElementDialog(this);
  if (!res.has_value())
    return;

  auto [kernel, borderType] = res.value();

  cv::Mat out;
  cv::morphologyEx(imageWrapper.getMat(), out, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), 1,
                   borderType);
  swapImage(out);
}

void MdiChild::morphologyClose() {
  auto res = structuringElementDialog(this);
  if (!res.has_value())
    return;

  auto [kernel, borderType] = res.value();

  cv::Mat out;
  cv::morphologyEx(imageWrapper.getMat(), out, cv::MORPH_CLOSE, kernel, cv::Point(-1, -1), 1,
                   borderType);
  swapImage(out);
}

void MdiChild::morphologySkeletonize() {
  auto res = structuringElementDialog(this);
  if (!res.has_value())
    return;

  auto [kernel, borderType] = res.value();

  cv::Mat out = imageProcessor::skeletonize(imageWrapper.getMat(), kernel, borderType);
  swapImage(out);
}

void MdiChild::houghTransform() {
  // TODO: add parameters for display
  auto res = houghLinesDialog(this);
  if (!res.has_value())
    return;

  auto params = res.value();

  const double theta = CV_PI * params.thetaDeg / 180.0;
  std::vector<cv::Vec4i> lines;

  std::vector<cv::Vec2f> linesPolar;
  cv::HoughLines(imageWrapper.getMat(), linesPolar, params.rho, theta, params.threshold);

  // Convert to endpoints (approx)
  for (const auto &l : linesPolar) {
    float r = l[0], t = l[1];
    double a = std::cos(t), b = std::sin(t);
    double x0 = a * r, y0 = b * r;
    int x1 = cvRound(x0 + 1000 * (-b));
    int y1 = cvRound(y0 + 1000 * (a));
    int x2 = cvRound(x0 - 1000 * (-b));
    int y2 = cvRound(y0 - 1000 * (a));
    lines.push_back(cv::Vec4i(x1, y1, x2, y2));
  }

  cv::Mat out = imageWrapper.getMat();
  for (const auto &l : lines)
    cv::line(out, {l[0], l[1]}, {l[2], l[3]}, cv::Scalar(255, 0, 0), 2);

  swapImage(out);
}
