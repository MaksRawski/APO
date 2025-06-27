#include "imageWrapper.hpp"
#include <QImage>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <optional>
#include <qpixmap.h>
#include <stdexcept>

namespace PixelFormatUtils {
PixelFormat fromChannelsNumber(int channels) {
  switch (channels) {
  case 1:
    return PixelFormat::Grayscale8;
  case 3:
    return PixelFormat::BGR24;
  default:
    throw std::runtime_error("Unsupported image format");
  }
}

int toCvType(const PixelFormat &format) {
  switch (format) {
  case PixelFormat::Binary:
  case PixelFormat::Grayscale8:
    return CV_8UC1;
  case PixelFormat::BGR24:
  case PixelFormat::HSV24:
  case PixelFormat::Lab24:
    return CV_8UC3;
  }
}

std::string toString(const PixelFormat &format) {
  switch (format) {
  case PixelFormat::Binary:
    return "Binary";
  case PixelFormat::Grayscale8:
    return "8-bit Grayscale";
  case PixelFormat::BGR24:
    // we will always have to convert BGR to RGB for display,
    // so it's fine to call this format just RGB
    return "RGB";
  case PixelFormat::HSV24:
    return "HSV";
  case PixelFormat::Lab24:
    return "Lab";
  }
}
std::vector<std::string> channelNames(const PixelFormat &format) {
  switch (format) {
  case PixelFormat::Binary:
  case PixelFormat::Grayscale8:
    return {};
  case PixelFormat::BGR24:
    // we will always have to convert BGR to RGB for display,
    // so it's fine to have these channel names as R,G,B
    return {"Red", "Green", "Blue"};
  case PixelFormat::HSV24:
    return {"Hue", "Saturation", "Value"};
  case PixelFormat::Lab24:
    return {"L", "a", "b"};
  }
}
} // namespace PixelFormatUtils

ImageWrapper::ImageWrapper(const ImageWrapper &other)
    : format_(other.format_), mat_(other.mat_.clone()) {}

ImageWrapper &ImageWrapper::operator=(const ImageWrapper &rhs) {
  mat_ = rhs.mat_.clone();
  format_ = rhs.format_;
  return *this;
}

ImageWrapper ImageWrapper::fromPath(QString filePath) {
  return ImageWrapper(cv::imread(filePath.toStdString(), cv::IMREAD_ANYCOLOR));
}

ImageWrapper::ImageWrapper(cv::Mat mat) : mat_(std::move(mat)) {
  format_ = PixelFormatUtils::fromChannelsNumber(mat_.channels());
}

QImage ImageWrapper::generateQImage() const {
  QImage img;
  switch (format_) {
  case PixelFormat::Binary:
  case PixelFormat::Grayscale8: {
    CV_Assert(mat_.type() == CV_8UC1);
    img = QImage(mat_.data, mat_.cols, mat_.rows, mat_.step, QImage::Format_Grayscale8).copy();
    break;
  }
  case PixelFormat::BGR24: {
    CV_Assert(mat_.type() == CV_8UC3);
    cv::Mat rgbMat;
    cv::cvtColor(mat_, rgbMat, cv::COLOR_BGR2RGB);
    img = QImage(rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step, QImage::Format_RGB888).copy();
    break;
  }

  case PixelFormat::HSV24: {
    CV_Assert(mat_.type() == CV_8UC3);
    cv::Mat rgbMat;
    cv::cvtColor(mat_, rgbMat, cv::COLOR_HSV2RGB);
    img = QImage(rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step, QImage::Format_RGB888).copy();
    break;
  }

  case PixelFormat::Lab24: {
    CV_Assert(mat_.type() == CV_8UC3);
    cv::Mat rgbMat;
    cv::cvtColor(mat_, rgbMat, cv::COLOR_Lab2RGB);
    img = QImage(rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step, QImage::Format_RGB888).copy();
    break;
  }
  }
  return img;
}

QPixmap ImageWrapper::generateQPixmap() const {
  QImage im = generateQImage();
  if (im.isNull()) {
    throw new std::runtime_error("Failed to generate a QImage!");
  }
  return QPixmap::fromImage(im);
}

std::vector<ImageWrapper> ImageWrapper::splitChannels() const {
  std::vector<cv::Mat> channels;
  cv::split(mat_, channels);

  std::vector<ImageWrapper> imageWrappers;
  for (auto channel : channels) {
    imageWrappers.push_back(ImageWrapper(channel));
  }
  // swap B and R channels
  if (format_ == PixelFormat::BGR24) {
    std::swap(imageWrappers[2], imageWrappers[0]);
  }
  return imageWrappers;
}

ImageWrapper ImageWrapper::toRGB() const {
  ImageWrapper out;
  switch (format_) {
  case PixelFormat::BGR24: {
    out = ImageWrapper(*this);
    break;
  }
  case PixelFormat::HSV24: {
    cv::Mat res;
    cv::cvtColor(mat_, res, cv::COLOR_HSV2BGR);
    out = ImageWrapper(res);
    break;
  }
  case PixelFormat::Lab24: {
    cv::Mat res;
    cv::cvtColor(mat_, res, cv::COLOR_Lab2BGR);
    out = ImageWrapper(res);
    break;
  }
  case PixelFormat::Binary:
  case PixelFormat::Grayscale8: {
    cv::Mat res;
    cv::cvtColor(mat_, res, cv::COLOR_GRAY2BGR);
    out = ImageWrapper(res);
    break;
  }
  default:
    throw new std::runtime_error("not yet implemented!");
  }
  out.format_ = PixelFormat::BGR24;
  return out;
}

ImageWrapper ImageWrapper::toHSV() const {
  ImageWrapper out;
  switch (format_) {
  case PixelFormat::BGR24: {
    cv::Mat res;
    cv::cvtColor(mat_, res, cv::COLOR_BGR2HSV);
    out = ImageWrapper(res);
    break;
  }
  case PixelFormat::HSV24: {
    out = ImageWrapper(*this);
    break;
  }
  case PixelFormat::Lab24: {
    cv::Mat bgr, res;
    cv::cvtColor(mat_, bgr, cv::COLOR_Lab2BGR);
    cv::cvtColor(bgr, res, cv::COLOR_BGR2HSV);
    out = ImageWrapper(res);
    break;
  }
  case PixelFormat::Binary:
  case PixelFormat::Grayscale8: {
    cv::Mat bgr, res;
    cv::cvtColor(mat_, bgr, cv::COLOR_GRAY2BGR);
    cv::cvtColor(bgr, res, cv::COLOR_BGR2HSV);
    out = ImageWrapper(res);
    break;
  }
  default:
    throw new std::runtime_error("not yet implemented!");
  }
  out.format_ = PixelFormat::HSV24;
  return out;
}

ImageWrapper ImageWrapper::toLab() const {
  ImageWrapper out;
  switch (format_) {
  case PixelFormat::BGR24: {
    cv::Mat res;
    cv::cvtColor(mat_, res, cv::COLOR_BGR2Lab);
    out = ImageWrapper(res);
    break;
  }
  case PixelFormat::HSV24: {
    cv::Mat bgr, res;
    cv::cvtColor(mat_, bgr, cv::COLOR_HSV2BGR);
    cv::cvtColor(bgr, res, cv::COLOR_BGR2Lab);
    out = ImageWrapper(res);
    break;
  }
  case PixelFormat::Lab24: {
    out = ImageWrapper(*this);
    break;
  }
  case PixelFormat::Binary:
  case PixelFormat::Grayscale8: {
    cv::Mat bgr, res;
    cv::cvtColor(mat_, bgr, cv::COLOR_GRAY2BGR);
    cv::cvtColor(bgr, res, cv::COLOR_BGR2Lab);
    out = ImageWrapper(res);
    break;
  }
  default:
    throw new std::runtime_error("not yet implemented!");
  }
  out.format_ = PixelFormat::Lab24;
  return out;
}

ImageWrapper ImageWrapper::toGrayscale() const {
  ImageWrapper out;
  switch (format_) {
  case PixelFormat::BGR24: {
    cv::Mat res;
    cv::cvtColor(mat_, res, cv::COLOR_BGR2GRAY);
    out = ImageWrapper(res);
    break;
  }
  case PixelFormat::HSV24: {
    cv::Mat bgr, res;
    cv::cvtColor(mat_, bgr, cv::COLOR_HSV2BGR);
    cv::cvtColor(bgr, res, cv::COLOR_BGR2GRAY);
    out = ImageWrapper(res);
    break;
  }
  case PixelFormat::Lab24: {
    cv::Mat bgr, res;
    cv::cvtColor(mat_, res, cv::COLOR_Lab2BGR);
    cv::cvtColor(bgr, res, cv::COLOR_BGR2GRAY);
    out = ImageWrapper(res);
    break;
  }
  case PixelFormat::Grayscale8:
  case PixelFormat::Binary:
  {
    out = ImageWrapper(*this);
    break;
  }
  default:
    throw new std::runtime_error("not yet implemented!");
  }
  out.format_ = PixelFormat::Grayscale8;
  return out;
}

// will return an ImageWrapper with a Binary format only if
// the image is grayscale and doesn't contain any other value than 0 and 255
std::optional<ImageWrapper> ImageWrapper::toBinary() const {
  if (format_ != PixelFormat::Grayscale8)
    return std::nullopt;

  double minVal, maxVal;
  cv::minMaxLoc(mat_, &minVal, &maxVal);

  if ((minVal != 0 && minVal != 255) || (maxVal != 0 && maxVal != 255))
    return std::nullopt;

  // check that all pixels are either 0 or 255
  cv::Mat nonZero, non255;
  cv::compare(mat_, 0, nonZero, cv::CMP_NE);
  cv::compare(mat_, 255, non255, cv::CMP_NE);
  cv::bitwise_and(nonZero, non255, nonZero);
  if (cv::countNonZero(nonZero) > 0)
    return std::nullopt;

  ImageWrapper out(*this);
  out.format_ = PixelFormat::Binary;
  return out;
}
