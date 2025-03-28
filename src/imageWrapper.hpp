#pragma once

#include <QImage>
#include <cstdint>
#include <opencv2/opencv.hpp>
#include <optional>
#include <vector>

enum class ImageType {
  Binary,
  GrayScale,
  RGB,
  HSV,
  Lab,
  ARGB,
};

constexpr std::optional<ImageType>
imageTypeFromQImageFormat(QImage::Format format) {
  switch (format) {
  case QImage::Format_Mono:
    return ImageType::Binary;
  case QImage::Format_Grayscale8:
    return ImageType::GrayScale;
  case QImage::Format_RGB888:
    return ImageType::RGB; // ARGB maps to RGB in qimageFormat()
  default:
    return std::nullopt; // unknown format or unsupported
  }
}

constexpr const char *toString(ImageType type) {
  switch (type) {
  case ImageType::Binary:
    return "binary";
  case ImageType::GrayScale:
    return "grayscale";
  case ImageType::RGB:
    return "RGB";
  case ImageType::HSV:
    return "HSV";
  case ImageType::Lab:
    return "Lab";
  case ImageType::ARGB:
    return "ARGB";
  }
}

constexpr uint8_t bitsPerPixel(ImageType type) {
  switch (type) {
  case ImageType::Binary:
    return 1;
  case ImageType::GrayScale:
    return 8;
  case ImageType::RGB:
  case ImageType::HSV:
  case ImageType::Lab:
    return 3 * 8;
  case ImageType::ARGB:
    return 4 * 8;
  }
}

// Returns a matching QImage::Format or nullopt if there isn't one.
constexpr std::optional<QImage::Format> qimageFormat(ImageType type) {
  switch (type) {
  case ImageType::Binary:
    return QImage::Format_Mono;
  case ImageType::GrayScale:
    return QImage::Format_Grayscale8;
  case ImageType::RGB:
    return QImage::Format_RGB888;
  case ImageType::ARGB:
    return QImage::Format_RGB888;
  case ImageType::HSV:
  case ImageType::Lab:
    return std::nullopt;
  }
}

constexpr int cvImageType(ImageType type) {
  switch (type) {
  case ImageType::Binary:
  case ImageType::GrayScale:
    return CV_8UC1;
  case ImageType::RGB:
  case ImageType::HSV:
  case ImageType::Lab:
    return CV_8UC3;
  case ImageType::ARGB:
    return CV_8UC4;
  }
}

class ImageWrapper {
public:
  ImageWrapper(std::vector<uchar> data, int width, int height, ImageType type);
  ImageWrapper(QImage image);
  // ImageWrapper(cv::Mat mat);

  const QImage &getQImage() const { return qimage_; };
  const cv::Mat &getCvMat() const { return mat_; };
  const std::vector<uchar> &getRawData() const { return data_; }

  int getWidth() const { return width_; }
  int getHeight() const { return height_; }
  ImageType getType() const { return type_; }

// signals:
//   void dataChanged();

private:
  int width_, height_;
  std::vector<uchar> data_;
  ImageType type_;
  cv::Mat mat_;
  QImage qimage_;
};
