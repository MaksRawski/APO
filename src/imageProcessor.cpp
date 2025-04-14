#include "imageProcessor.hpp"
#include "imageWrapper.hpp"
#include <QColorSpace>
#include <qimage.h>
#include <stdexcept>
#include <vector>

namespace imageProcessor {
std::vector<int> histogram(const ImageWrapper &imageWrapper) {
  cv::Mat mat = imageWrapper.getMat();
  return histogram(mat);
}

std::vector<int> histogram(const cv::Mat &mat) {
  // we only calculate histograms for grayscale images
  if (mat.type() != CV_8UC1)
    return {};

  std::vector<int> histogram(M, 0);

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
cv::Mat applyLUTcv(const cv::Mat &mat, const LUT &lut){
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
  LUT lut(M);
  lut.resize(M);
  for (int i = 0; i < M; ++i) {
    lut[i] = LMAX - i - 1;
  }
  return lut;
}

LUT stretch(uchar p1, uchar p2, uchar q3, uchar q4) {
  LUT lut(M);
  double stretchFactor = static_cast<double>(q4 - q3) / (p2 - p1);
  for (int i = 0; i < M; ++i) {
    if (p1 <= i && i <= p2)
      lut[i] = static_cast<uchar>(double(i - p1) * stretchFactor) + q3;
    else
      lut[i] = i;
  }
  return lut;
}

LUT posterize(uchar n) {
  LUT lut(M);
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

LUT equalize(const cv::Mat &mat) {
  std::vector<int> hist = histogram(mat);

  if (mat.channels() != 1) {
    throw std::runtime_error(
        "Tried to create an equalization LUT for non-grayscale image!");
  }
  int s = 0;
  std::vector<int> cdf(256);

  for (int i = 0; i < 256; ++i) {
    s += hist[i];
    cdf[i] = s;
  }

  // equalize cdf
  int totalPixels = mat.rows * mat.cols;

  // calculate equalization LUT
  LUT lut(256);
  for (int i = 0; i < 256; ++i) {
    lut[i] = static_cast<uchar>(255.0 * cdf[i] / totalPixels);
  }
  return lut;
}

cv::Mat equalizeChannels(const cv::Mat &image) {
  std::vector<cv::Mat> channels;
  cv::split(image, channels);

  for (int i = 0; i < channels.size(); ++i) {
    std::vector<uchar> lut = equalize(channels[i]);
    channels[i] = applyLUTcv(channels[i], lut);
  }

  cv::Mat equalizedImage;
  cv::merge(channels, equalizedImage);

  return equalizedImage;
}

} // namespace imageProcessor
