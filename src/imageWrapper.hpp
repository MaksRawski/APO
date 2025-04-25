#pragma once

#include <QImage>
#include <opencv2/opencv.hpp>
#include <qimage.h>

namespace PixelFormatUtils {
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
};
PixelFormat fromChannelsNumber(int channels);
int toCvType(const PixelFormat &format);
std::string toString(const PixelFormat &format);
std::vector<std::string> channelNmaes(const PixelFormat &format);
}

using PixelFormatUtils::PixelFormat;

class ImageWrapper {
public:
  ImageWrapper() = default;
  ImageWrapper(const ImageWrapper &imageWrapper);
  ImageWrapper &operator=(const ImageWrapper &rhs);
  ImageWrapper(cv::Mat mat);
  static ImageWrapper fromPath(QString filePath);

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
