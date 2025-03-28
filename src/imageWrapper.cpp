#include "imageWrapper.hpp"
#include <vector>

ImageWrapper::ImageWrapper(std::vector<uchar> data, int width, int height,
                           ImageType type)
    : width_(width), height_(height), data_(data), type_(type) {

  mat_ = cv::Mat(height_, width_, cvImageType(type), data.data());
}

ImageWrapper::ImageWrapper(QImage image) : qimage_(image) {
  width_ = image.width();
  height_ = image.height();
  auto type = imageTypeFromQImageFormat(image.format());
  if (!type.has_value()) {
    throw std::runtime_error("Unsupported QImage format: " +
                             std::to_string(static_cast<int>(image.format())));
  }

  type_ = type.value();
  mat_ = cv::Mat(height_, width_, cvImageType(type_), qimage_.bits());
}
