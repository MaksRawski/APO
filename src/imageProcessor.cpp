#include "imageProcessor.hpp"
#include "imageWrapper.hpp"
#include <QColorSpace>
#include <qimage.h>
#include <vector>

namespace imageProcessor {
std::vector<int> histogram(const ImageWrapper &imageWrapper) {
  cv::Mat image = imageWrapper.getMat();

  // we don't display histograms for non grayscale images
  if (image.type() != CV_8UC1)
    return {};

  std::vector<int> histogram(M, 0);

  for (int y = 0; y < image.rows; ++y) {
    const uchar *rowPtr = image.ptr<uchar>(y);
    for (int x = 0; x < image.cols; ++x) {
      histogram[rowPtr[x]]++;
    }
  }
  return histogram;
}

LUT negate() {
  LUT lut;
  lut.resize(M);
  for (int i = 0; i < M; ++i) {
    lut[i] = LMAX - i;
  }
  return lut;
}

LUT stretch(int p1, int p2, int q3, int q4) {
  LUT lut;
  lut.resize(M);
  double div = 1.0 / static_cast<double>(p2 - p1);
  for (size_t i = 0; i < M; ++i) {
    if (i < p1)
      lut[i] = q3;
    else if (i <= p2)
      lut[i] = (i - p1) * q4 * div;
    else
      lut[i] = q4;
  }
  return lut;
}

LUT posterize(int n) {
  LUT lut;
  lut.resize(M);
  std::vector<int> colors;
  colors.resize(n);

  for (int i = 0; i < n; ++i) {
    colors[i] = LMAX / i;
  }
  for (int i = 0; i <= M; ++i) {
    lut[i] = colors[i / n];
  }
  return lut;
}

QImage toGrayScale(const QImage &image) {
  return image.convertToFormat(QImage::Format_Grayscale8);
}
} // namespace imageProcessor
