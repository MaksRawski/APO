#pragma once

#include "imageWrapper.hpp"
#include <QImage>
#include <cstdint>
#include <vector>

namespace imageProcessor {
using LUT = std::vector<uchar>;

const int LMIN = 0;
const int LMAX = 256;
const int M = LMAX - LMIN;

ImageWrapper applyLUT(const ImageWrapper &image, const LUT &lut);
std::vector<int> histogram(const ImageWrapper &image);
LUT negate();
LUT stretch(int p1, int p2, int q3, int q4);
LUT posterize(uint8_t n);
} // namespace imageProcessor
