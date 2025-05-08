#include "imageWrapper.hpp"
#include <QImage>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
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
std::vector<std::string> channelNmaes(const PixelFormat &format) {
  switch (format) {
  case PixelFormat::Binary:
  case PixelFormat::Grayscale8:
    return {};
  case PixelFormat::BGR24:
    // we will always have to convert BGR to RGB for display,
    // so it's fine to have theses channel names as R,G,B
    return {"R", "G", "B"};
  case PixelFormat::HSV24:
    return {"Hue", "Saturation", "Value"};
  case PixelFormat::Lab24:
    return {"L", "a", "b"};
  }
}
} // namespace PixelFormatUtils

ImageWrapper::ImageWrapper(const ImageWrapper &other) : format_(other.format_), mat_(other.mat_.clone()) {}

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
    img = QImage(mat_.data, mat_.cols, mat_.rows, mat_.step,
                 QImage::Format_Grayscale8)
              .copy();
    break;
  }
  case PixelFormat::BGR24: {
    CV_Assert(mat_.type() == CV_8UC3);
    cv::Mat rgbMat;
    cv::cvtColor(mat_, rgbMat, cv::COLOR_BGR2RGB);
    img = QImage(rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step,
                 QImage::Format_RGB888)
              .copy();
    break;
  }

  case PixelFormat::HSV24: {
    CV_Assert(mat_.type() == CV_8UC3);
    cv::Mat rgbMat;
    cv::cvtColor(mat_, rgbMat, cv::COLOR_HSV2RGB);
    img = QImage(rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step,
                 QImage::Format_RGB888)
              .copy();
    break;
  }

  case PixelFormat::Lab24: {
    CV_Assert(mat_.type() == CV_8UC3);
    cv::Mat rgbMat;
    cv::cvtColor(mat_, rgbMat, cv::COLOR_Lab2RGB);
    img = QImage(rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step,
                 QImage::Format_RGB888)
              .copy();
    break;
  }
  }
  return img;
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
  case PixelFormat::Grayscale8: {
    out = ImageWrapper(*this);
    break;
  }
  default:
    throw new std::runtime_error("not yet implemented!");
  }
  out.format_ = PixelFormat::Grayscale8;
  return out;
}
