#pragma once

#include <QImage>
#include <opencv2/opencv.hpp>
#include <qimage.h>
#include <stdexcept>
#include <string>

// For internal storage in ImageWrapper.
enum class PixelFormat {
  // 8-bits per pixel, either all 0s or 1s
  Binary,
  // 8-bits per pixel
  Grayscale8,
  // 24-bits per pixel (0xBBGGRR)
  BGR24,
  // 24-bits per pixel (0xHHSSVV)
  HSV24,
  // 24-bits per pixel (0xLLaabb)
  Lab24,
  // 32-bits per pixel (0xBBGGRRAA)
  BGRA32,
};

constexpr PixelFormat pixelFormatFromChannelsNumber(int channels) {
  switch (channels) {
  case 1:
    return PixelFormat::Grayscale8;
  case 3:
    return PixelFormat::BGR24;
  case 4:
    return PixelFormat::BGRA32;
  default:
    throw std::runtime_error("Unsupported image format");
  }
}

constexpr int pixelFormatToCvType(PixelFormat format) {
  switch (format) {
  case PixelFormat::Binary:
  case PixelFormat::Grayscale8:
    return CV_8UC1;
  case PixelFormat::BGR24:
  case PixelFormat::HSV24:
  case PixelFormat::Lab24:
    return CV_8UC3;
  case PixelFormat::BGRA32:
    return CV_8UC4;
  }
}

constexpr const char *pixelFormatToString(PixelFormat format) {
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
  case PixelFormat::BGRA32:
    // we will always have to convert BGRA to RGBA for display,
    // so it's fine to call this format just RGBA
    return "RGBA";
  }
}

class ImageWrapper {
public:
  ImageWrapper() = default;
  ImageWrapper(const ImageWrapper &imageWrapper);
  ImageWrapper &operator=(const ImageWrapper &rhs);
  ImageWrapper(QString filePath);
  ImageWrapper(cv::Mat mat);

  const cv::Mat &getMat() const { return mat_; }
  int getWidth() const { return mat_.cols; }
  int getHeight() const { return mat_.rows; }
  PixelFormat getFormat() const { return format_; }
  QImage generateQImage() const;

  std::vector<ImageWrapper> splitChannels() const;
  ImageWrapper toRGB() const;
  ImageWrapper toHSV() const;
  ImageWrapper toLab() const;
  ImageWrapper toGrayscale() const;

signals:
  void dataChanged(QPixmap pixmap);

private:
  PixelFormat format_;
  cv::Mat mat_;
};
