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
cv::Mat applyLUTcv(const cv::Mat &mat, const LUT &lut);
std::vector<int> histogram(const ImageWrapper &image);
std::vector<int> histogram(const cv::Mat &mat);
LUT negate();
LUT stretch(uchar p1, uchar p2, uchar q3, uchar q4);
LUT posterize(uchar n);
LUT equalize(const cv::Mat &mat);
cv::Mat equalizeChannels(const cv::Mat &mat);
} // namespace imageProcessor
