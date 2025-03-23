#include "imageProcessor.hpp"

namespace imageProcessor {
LUT histogram(const QImage &image) {
  std::vector<int> lut;
  lut.resize(M);

  for (int r = 0; r < image.height(); ++r) {
    for (int c = 0; c < image.width(); ++c) {
      ++lut[image.pixelColor(r, c).lightness()];
    }
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
  float div = 1.0f / float(p2 - p1);
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
} // namespace imageProcessor
