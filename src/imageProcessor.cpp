#include "imageProcessor.hpp"
#include "imageWrapper.hpp"
#include <QColorSpace>
#include <opencv2/core.hpp>
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
  for (int i = 0; i < 256; ++i) {
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

std::vector<uchar> equalize(const cv::Mat &mat) {
  std::vector<int> hist = histogram(mat);

  if (mat.channels() != 1) {
    throw std::runtime_error("Tried to create an equalization LUT for non-grayscale image!");
  }
  std::vector<float> cdf(256, 0);
  int totalPixels = mat.rows * mat.cols;
  cdf[0] = static_cast<float>(hist[0]) / totalPixels;
  for (int i = 1; i < 256; ++i) {
    cdf[i] = cdf[i - 1] + static_cast<float>(hist[i]) / totalPixels;
  }

  std::vector<uchar> lut(256, 0);
  for (int i = 0; i < 256; ++i) {
    lut[i] = static_cast<uchar>(std::round(cdf[i] * 255));
  }

  return lut;
}

cv::Mat equalizeChannels(const cv::Mat &image) {
  std::vector<cv::Mat> channels;
  cv::split(image, channels);

  for (uchar i = 0; i < channels.size(); ++i) {
    std::vector<uchar> lut = equalize(channels[i]);
    channels[i] = applyLUTcv(channels[i], lut);
  }

  cv::Mat equalizedImage;
  cv::merge(channels, equalizedImage);

  return equalizedImage;
}

cv::Mat medianBlur(const cv::Mat &image, int k, int borderType) {
  int pad = k / 2;
  std::vector<cv::Mat> mats;
  std::vector<cv::Mat> outs;
  cv::split(image, mats);

  for (cv::Mat mat : mats) {
    cv::Mat out;
    cv::copyMakeBorder(mat, out, pad, pad, pad, pad, borderType);

    int radius = k / 2;
    if (mat.channels() == 1) {
      for (int y = radius; y < out.rows - radius; ++y) {
        uchar *rowPtr = out.ptr(y);
        for (int x = radius; x < out.cols - radius; ++x) {
          std::vector<uchar> neighbors;

          for (int i = -radius; i <= radius; ++i) {
            for (int j = -radius; j <= radius; ++j) {
              uchar val = out.at<uchar>(y + i, x + j);
              neighbors.push_back(val);
            }
          }

          std::nth_element(neighbors.begin(), neighbors.begin() + neighbors.size() / 2,
                           neighbors.end());

          rowPtr[x] = neighbors[neighbors.size() / 2];
        }
      }
    }
    outs.push_back(out);
  }
  cv::Mat out;
  cv::merge(outs, out);

  return out;
}

} // namespace imageProcessor
