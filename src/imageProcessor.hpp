#pragma once

#include "imageWrapper.hpp"
#include <QImage>
#include <cstdint>
#include <vector>

namespace imageProcessor {
using LUT = std::vector<uchar>;

ImageWrapper applyLUT(const ImageWrapper &image, const LUT &lut);
cv::Mat applyLUTcv(const cv::Mat &mat, const LUT &lut);
std::vector<int> histogram(const ImageWrapper &image);
std::vector<int> histogram(const cv::Mat &mat);
LUT negate();
LUT stretch(uchar p1, uchar p2, uchar q3, uchar q4);
LUT posterize(uchar n);
LUT equalize(const cv::Mat &mat);
cv::Mat medianBlur(const cv::Mat &mat, int k, int borderType);
cv::Mat equalizeChannels(const cv::Mat &mat);
cv::Mat operateMats(const cv::Mat &first, const cv::Mat &second, uchar (*op)(uchar, uchar));
} // namespace imageProcessor
