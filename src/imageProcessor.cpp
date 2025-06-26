#include "imageProcessor.hpp"
#include "imageWrapper.hpp"
#include <QColorSpace>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
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

namespace {
cv::Mat convolveNormalize(cv::Mat image, cv::Mat kernel, int borderType) {
  cv::Mat kernelScaled;
  kernel.convertTo(kernelScaled, CV_32FC1);
  kernelScaled /= cv::sum(kernel)[0];

  return imageProcessor::applyToChannels(image, [kernelScaled, borderType](cv::Mat channel) {
    cv::Mat convertedChannel;
    channel.convertTo(convertedChannel, CV_32FC1);

    cv::Mat temp;
    cv::filter2D(convertedChannel, temp, CV_32FC1, kernelScaled, cv::Point(-1, -1), 0, borderType);

    cv::Mat out;
    cv::convertScaleAbs(temp, out);
    return out;
  });
}
} // namespace

cv::Mat convolve(cv::Mat image, cv::Mat kernel, int borderType) {
  int sum = cv::sum(kernel)[0];
  if (sum == 0 || sum == 1) {
    cv::Mat out;
    // TODO: should probably ensure that kernel is CV_32F?
    cv::filter2D(image, out, -1, kernel, cv::Point(-1, -1), 0, borderType);
    return out;
  } else {
    return convolveNormalize(image, kernel, borderType);
  }
}
std::vector<uchar> extractLineProfile(const cv::Mat &img, cv::Point p1, cv::Point p2) {
  std::vector<uchar> profile;

  cv::LineIterator it(img, p1, p2, 8);
  for (int i = 0; i < it.count; i++, ++it) {
    profile.push_back(**it);
  }

  return profile;
}

namespace {
// Affine transformation is by definition:
// T = AX + B, where:
//    T - result of a transformation
//    X - input
//    A, B - transformation parameters
//
// in case where X is a single point:
// X = [x y]
// A = [a00 a01]
//     [a10 a11]
// B = [b1]
//     [b2]
//
// we can simplify the transformation equation to be in the form of matrix multiplication:
//
//   T  = [   A    B ] X
//                    [x]
// [Tx] = [a00 a01 b1][y]
// [Ty]   [a10 a11 b2][1]
//
// For this equation to have a unique solution, given T and X, X must be a square matrix.
// Plugging 3 points as input gives:
//
//       T       = [   A    B ]      X
//                              [x1 x2 x3]
// [Tx1 Tx2 Tx3] = [a00 a01 b1] [y1 y2 y3]
// [Ty1 Ty2 Ty3]   [a10 a11 b2] [ 1  1  1]
cv::Mat getAffineMatrix(std::vector<cv::Point2f> srcPoints, std::vector<cv::Point2f> dstPoints) {
  assert(srcPoints.size() == 3 && "There must be exactly 3 points");

  //
  cv::Mat X = (cv::Mat_<double>(3, 3) << srcPoints[0].x, srcPoints[1].x, srcPoints[2].x,
               srcPoints[0].y, srcPoints[1].y, srcPoints[2].y, 1, 1, 1);
  cv::Mat T = (cv::Mat_<double>(2, 3) << dstPoints[0].x, dstPoints[1].x, dstPoints[2].x,
               dstPoints[0].y, dstPoints[1].y, dstPoints[2].y);

  if (cv::determinant(X) == 0) {
    qDebug() << "provided points form a line";
    return cv::Mat_<double>(2, 3) << 0, 0, 0, 0, 0, 0;
  }

  cv::Mat invX;
  cv::invert(X, invX);

  cv::Mat M = T * invX;

  return M;
}
// we want each pixel in the destination image to be moved according to the affine matrix, so:
// dst(x,y) = src( M(0,0)x + M(0,1)y + M(0,2),
//                 M(1,0)x + M(1,1)y + M(1,2) )
// so for each point in DESTINATION image we must find corresponding point in the SOURCE image
//
// since
// T = MX, then
// X = M'T where
//   M' is the inverse of an affine transformation given by M
//
// M' can be calculated using cv::invertAffineTransform
//
// then M' must simply be applied to each destination's point coordinates to find out the coordinates
// of that point in source. As the result of the mapping will rarely be a whole number, the value of
// a destination point will be a bilinear interpolation of 4 points in the source image that are neighbors of the
// closest point.
cv::Mat warpAffine(const cv::Mat &mat, const cv::Mat &affineMat) {
  cv::Mat invAffine; // M'
  cv::invertAffineTransform(affineMat, invAffine);

  double a00 = invAffine.at<double>(0, 0);
  double a01 = invAffine.at<double>(0, 1);
  double b1 = invAffine.at<double>(0, 2);
  double a10 = invAffine.at<double>(1, 0);
  double a11 = invAffine.at<double>(1, 1);
  double b2 = invAffine.at<double>(1, 2);

  cv::Mat dst = cv::Mat::zeros(mat.rows, mat.cols, mat.type());

  for (int y = 0; y < dst.rows; ++y) {
    for (int x = 0; x < dst.cols; ++x) {
      float srcX = a00 * x + a01 * y + b1;
      float srcY = a10 * x + a11 * y + b2;

      int x0 = static_cast<int>(std::floor(srcX));
      int y0 = static_cast<int>(std::floor(srcY));

      float dx = srcX - x0;
      float dy = srcY - y0;

      // if not on a bottom or right border
      if (x0 >= 0 && x0 + 1 < mat.cols && y0 >= 0 && y0 + 1 < mat.rows) {
        switch (mat.channels()) {
        case 1: {
          // value of the current pixel is a bilinear interpolation of values of
          // 4 neighboring pixels (with the current one being the top-left one)
          float v00 = mat.at<uchar>(y0, x0);
          float v01 = mat.at<uchar>(y0, x0 + 1);
          float v10 = mat.at<uchar>(y0 + 1, x0);
          float v11 = mat.at<uchar>(y0 + 1, x0 + 1);

          // The actual value that will be put in place of this pixel is going to be
          // interpolated based on the real distance from the closest pixel.
          // The closer the neighboring point is the closer to it the value will be.
          float val =
              (1 - dx) * (1 - dy) * v00 + dx * (1 - dy) * v01 + (1 - dx) * dy * v10 + dx * dy * v11;

          dst.at<uchar>(y, x) = cv::saturate_cast<uchar>(val);
          break;
        };
        case 3: {
          for (int c = 0; c < 3; ++c) {
            float v00 = mat.at<cv::Vec3b>(y0, x0)[c];
            float v01 = mat.at<cv::Vec3b>(y0, x0 + 1)[c];
            float v10 = mat.at<cv::Vec3b>(y0 + 1, x0)[c];
            float v11 = mat.at<cv::Vec3b>(y0 + 1, x0 + 1)[c];

            float val = (1 - dx) * (1 - dy) * v00 + dx * (1 - dy) * v01 + (1 - dx) * dy * v10 +
                        dx * dy * v11;

            dst.at<cv::Vec3b>(y, x)[c] = cv::saturate_cast<uchar>(val);
          }
          break;
        }
        }
      }
    }
  }
  return dst;
}
} // namespace

cv::Mat affineTransform(const cv::Mat &mat, std::vector<cv::Point2f> srcPoints,
                        std::vector<cv::Point2f> dstPoints) {
  // cv::Mat warpMat = cv::getAffineTransform(srcPoints, dstPoints);
  cv::Mat warpMat = getAffineMatrix(srcPoints, dstPoints);

  // cv::Mat dst;
  // cv::warpAffine(mat, dst, warpMat, mat.size());
  // return dst;
  return warpAffine(mat, warpMat);
}

} // namespace imageProcessor
