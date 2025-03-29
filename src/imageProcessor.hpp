#pragma once

#include "imageWrapper.hpp"
#include <QImage>
#include <cstdint>
#include <vector>

namespace imageProcessor {
using LUT = std::vector<int>;

const int LMIN = 0;
const int LMAX = 256;
const int M = LMAX - LMIN;

LUT histogram(const ImageWrapper &image);
LUT negate();
LUT stretch(int p1, int p2, int q3, int q4);
LUT posterize(uint8_t n);

QImage toGrayScale(const QImage &image);
} // namespace imageProcessor
