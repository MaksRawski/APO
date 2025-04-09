#include "imageWrapper.hpp"
#include <opencv2/imgcodecs.hpp>
#include <QImage>

ImageWrapper::ImageWrapper(const ImageWrapper &other): format_(other.format_) {
  other.mat_.copyTo(this->mat_);
}

ImageWrapper &ImageWrapper::operator=(const ImageWrapper &rhs)  {
  rhs.mat_.copyTo(this->mat_);
  format_ = rhs.format_;
  return *this;
}

ImageWrapper::ImageWrapper(QString filePath)
    : ImageWrapper(cv::imread(filePath.toStdString(), cv::IMREAD_ANYCOLOR)) {}

ImageWrapper::ImageWrapper(cv::Mat mat) : mat_(std::move(mat)) {
  format_ = pixelFormatFromChannelsNumber(mat_.channels());
}

QImage ImageWrapper::generateQImage() const {
  QImage img;
  switch (format_) {
  case PixelFormat::Binary:
  case PixelFormat::Grayscale8: {
    CV_Assert(mat_.type() == CV_8UC1);
    img = QImage(mat_.data, mat_.cols, mat_.rows, mat_.step,
                 QImage::Format_Grayscale8).copy();
    break;
  }
  case PixelFormat::BGR24: {
    CV_Assert(mat_.type() == CV_8UC3);
    cv::Mat rgbMat;
    cv::cvtColor(mat_, rgbMat, cv::COLOR_BGR2RGB);
    img = QImage(rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step,
                 QImage::Format_RGB888).copy();
    break;
  }

  case PixelFormat::HSV24: {
    CV_Assert(mat_.type() == CV_8UC3);
    cv::Mat rgbMat;
    cv::cvtColor(mat_, rgbMat, cv::COLOR_HSV2RGB);
    img = QImage(rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step,
                 QImage::Format_RGB888).copy();
    break;
  }

  case PixelFormat::Lab24: {
    CV_Assert(mat_.type() == CV_8UC3);
    cv::Mat rgbMat;
    cv::cvtColor(mat_, rgbMat, cv::COLOR_Lab2RGB);
    img = QImage(rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step,
                 QImage::Format_RGB888).copy();
    break;
  }

  case PixelFormat::BGRA32: {
    CV_Assert(mat_.type() == CV_8UC4);
    cv::Mat rgbaMat;
    cv::cvtColor(mat_, rgbaMat, cv::COLOR_BGRA2RGBA);
    img = QImage(rgbaMat.data, rgbaMat.cols, rgbaMat.rows, rgbaMat.step,
                 QImage::Format_RGB888).copy();
    break;
  }
  }
  return img;
}

std::vector<ImageWrapper> ImageWrapper::splitChannels() const {
  std::vector<cv::Mat> channels;
  cv::split(mat_, channels);
  std::vector<ImageWrapper> result;
  for (int i = 0; i < 3; ++i) {
    result.emplace_back(ImageWrapper(channels[i]));
  }
  return result;
}
