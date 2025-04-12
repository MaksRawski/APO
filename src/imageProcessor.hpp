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
// ImageWrapper applyLUT(const cv::Mat &mat, const LUT &lut);
cv::Mat applyLUTcv(const cv::Mat &mat, const LUT &lut);
std::vector<int> histogram(const ImageWrapper &image);
std::vector<int> histogram(const cv::Mat &mat);
LUT negate();
LUT stretch(int p1, int p2, int q3, int q4);
LUT posterize(uint8_t n);
// LUT equalize(const ImageWrapper &image);
LUT equalize(const cv::Mat &mat);
cv::Mat equalizeChannels(const cv::Mat &mat);
} // namespace imageProcessor
