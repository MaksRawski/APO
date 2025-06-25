#pragma once

#include "imageWrapper.hpp"
#include <QImage>
#include <vector>

namespace imageProcessor {
using LUT = std::vector<uchar>;

ImageWrapper applyLUT(const ImageWrapper &image, const LUT &lut);
cv::Mat applyLUTcv(const cv::Mat &mat, const LUT &lut);
std::vector<int> histogram(const cv::Mat &mat);
LUT negate();
LUT stretch(uchar p1, uchar p2, uchar q3, uchar q4);
LUT posterize(uchar n);
LUT equalizeLUT(const cv::Mat &mat);
cv::Mat medianBlur(const cv::Mat &mat, int k, int borderType);
cv::Mat applyToChannels(const cv::Mat &mat, std::function<cv::Mat(cv::Mat)> f);
cv::Mat normalizeChannels(const cv::Mat &mat);
cv::Mat equalizeChannels(const cv::Mat &mat);
cv::Mat rangeStretchChannels(const cv::Mat &mat, uchar p1, uchar p2, uchar q3, uchar q4);
cv::Mat skeletonize(const cv::Mat &mat, const cv::Mat &structuringElement, int borderType);
cv::Mat convolve(cv::Mat image, cv::Mat kernel, int borderType);
std::vector<uchar> extractLineProfile(const cv::Mat &img, cv::Point p1, cv::Point p2);
cv::Mat affineTransform(const cv::Mat &mat, std::vector<cv::Point2f> srcPoints,
                        std::vector<cv::Point2f> dstPoints);
} // namespace imageProcessor
