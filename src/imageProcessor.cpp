#include "imageProcessor.hpp"
#include "imageWrapper.hpp"
#include <QColorSpace>
#include <opencv2/core.hpp>
#include <qimage.h>
#include <stdexcept>
#include <vector>

namespace imageProcessor {
std::vector<int> histogram(const cv::Mat &mat) {
  // we only calculate histograms for grayscale images
  if (mat.type() != CV_8UC1)
    return {};

  std::vector<int> histogram(256, 0);

  for (int y = 0; y < mat.rows; ++y) {
    const uchar *rowPtr = mat.ptr<uchar>(y);
    for (int x = 0; x < mat.cols; ++x) {
      histogram[rowPtr[x]]++;
    }
  }
  return histogram;
}

ImageWrapper applyLUT(const ImageWrapper &image, const LUT &lut) {
  return ImageWrapper(applyLUTcv(image.getMat(), lut));
}

// applies LUT to every channel of an image
cv::Mat applyLUTcv(const cv::Mat &mat, const LUT &lut) {
  cv::Mat res = mat.clone();

  if (res.channels() == 1) {
    for (int y = 0; y < res.rows; ++y) {
      uchar *rowPtr = res.ptr<uchar>(y);
      for (int x = 0; x < res.cols; ++x) {
        rowPtr[x] = lut[rowPtr[x]];
      }
    }
  } else if (res.channels() == 3) { // Color (BGR)
    for (int y = 0; y < res.rows; ++y) {
      cv::Vec3b *rowPtr = res.ptr<cv::Vec3b>(y);
      for (int x = 0; x < res.cols; ++x) {
        rowPtr[x][0] = lut[rowPtr[x][0]]; // Blue
        rowPtr[x][1] = lut[rowPtr[x][1]]; // Green
        rowPtr[x][2] = lut[rowPtr[x][2]]; // Red
      }
    }
  } else if (res.channels() == 4) { // Color (BGRA)
    for (int y = 0; y < res.rows; ++y) {
      cv::Vec4b *rowPtr = res.ptr<cv::Vec4b>(y);
      for (int x = 0; x < res.cols; ++x) {
        rowPtr[x][0] = lut[rowPtr[x][0]]; // Blue
        rowPtr[x][1] = lut[rowPtr[x][1]]; // Green
        rowPtr[x][2] = lut[rowPtr[x][2]]; // Red
        rowPtr[x][3] = lut[rowPtr[x][3]]; // Alpha
      }
    }
  } else {
    throw std::runtime_error("Unsupported number of channels.");
  }
  return res;
}

LUT negate() {
  LUT lut(256);
  for (uint i = 0; i < 256; ++i) {
    lut[i] = 256 - i - 1;
  }
  return lut;
}

LUT stretch(uchar p1, uchar p2, uchar q3, uchar q4) {
  LUT lut(256);
  double stretchFactor = static_cast<double>(q4 - q3) / (p2 - p1);
  for (int i = 0; i < 256; ++i) {
    if (p1 <= i && i <= p2)
      lut[i] = static_cast<uchar>(double(i - p1) * stretchFactor) + q3;
    else
      lut[i] = i;
  }
  return lut;
}

LUT posterize(uchar n) {
  LUT lut(256);

  float step = 256.0f / n;
  for (int i = 0; i < 256; ++i) {
    uchar bin = static_cast<uchar>(i / step);
    lut[i] = static_cast<uchar>(bin * step + step / 2);
  }
  return lut;
}

LUT equalizeLUT(const cv::Mat &mat) {
  std::vector<int> hist = histogram(mat);

  if (mat.channels() != 1) {
    throw std::runtime_error("Tried to create an equalization LUT for a non-grayscale image!");
  }
  std::vector<float> cdf(256, 0);
  int totalPixels = mat.rows * mat.cols;
  cdf[0] = static_cast<float>(hist[0]) / totalPixels;
  for (int i = 1; i < 256; ++i) {
    cdf[i] = cdf[i - 1] + static_cast<float>(hist[i]) / totalPixels;
  }

  LUT lut(256, 0);
  for (int i = 0; i < 256; ++i) {
    lut[i] = static_cast<uchar>(std::round(cdf[i] * 255));
  }

  return lut;
}

cv::Mat applyToChannels(const cv::Mat &mat, std::function<cv::Mat(const cv::Mat)> f) {
  std::vector<cv::Mat> channels;
  cv::split(mat, channels);

  for (auto &channel : channels) {
    channel = f(channel);
  }

  cv::Mat out;
  cv::merge(channels, out);
  return out;
}

cv::Mat normalizeChannels(const cv::Mat &mat) {
  return applyToChannels(mat, [](const cv::Mat &channel) {
    double min, max;
    cv::minMaxLoc(channel, &min, &max);
    LUT stretched =
        imageProcessor::stretch(static_cast<uchar>(min), static_cast<uchar>(max), 0, 255);
    return applyLUTcv(channel, stretched);
  });
}

cv::Mat equalizeChannels(const cv::Mat &mat) {
  return applyToChannels(
      mat, [](const cv::Mat &channel) { return applyLUTcv(channel, equalizeLUT(channel)); });
}
cv::Mat rangeStretchChannels(const cv::Mat &mat, uchar p1, uchar p2, uchar q3, uchar q4) {
  return applyToChannels(mat, [=](const cv::Mat &channel) {
    return applyLUTcv(channel, imageProcessor::stretch(p1, p2, q3, q4));
  });
}

cv::Mat skeletonize(const cv::Mat &mat, const cv::Mat &structuringElement, int borderType) {
  return applyToChannels(mat, [structuringElement, borderType](cv::Mat channel) {
    cv::Mat skel(channel.size(), CV_8UC1, cv::Scalar(0));
    cv::Mat temp;
    cv::Mat eroded;
    cv::Mat img = channel.clone();

    while (true) {
      cv::erode(img, eroded, structuringElement, cv::Point(-1, -1), 1, borderType);
      cv::dilate(eroded, temp, structuringElement, cv::Point(-1, -1), 1, borderType);
      cv::subtract(img, temp, temp);
      cv::bitwise_or(skel, temp, skel);
      eroded.copyTo(img);

      if (cv::countNonZero(img) == 0)
        break;
    }

    return skel;
  });
}
} // namespace imageProcessor
