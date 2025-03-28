#include "imageProcessor.hpp"
#include "imageWrapper.hpp"
#include <qimage.h>
#include <QColorSpace>

namespace imageProcessor {
LUT histogram(const ImageWrapper &image) {
  auto data = image.getRawData();
  std::vector<int> lut;
  lut.resize(M);

  for (uchar v : data) {
      ++lut[v];
  }
  return lut;
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
  double div = 1.0f / static_cast<double>(p2 - p1);
  for (int i = 0; i < M; ++i) {
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

  QImage toGrayScale(const QImage &image){
    return image.convertToFormat(QImage::Format_Grayscale8);
  }
} // namespace imageProcessor
