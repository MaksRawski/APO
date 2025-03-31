#include <QImage>
#include <gtest/gtest.h>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <qpixmap.h>

#include "../src/imageWrapper.hpp"

// Test fixture for ImageWrapper
class ImageWrapperTest : public ::testing::Test {
protected:
  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(ImageWrapperTest, GenerateQImage_Grayscale) {
  cv::Mat mat(10, 20, CV_8UC1, cv::Scalar(42));

  ImageWrapper imageWrapper(mat);
  QImage image = imageWrapper.generateQImage();

  EXPECT_EQ(image.width(), mat.cols);
  EXPECT_EQ(image.height(), mat.rows);

  for (int y = 0; y < image.height(); ++y) {
    for (int x = 0; x < image.width(); ++x) {
      QRgb pixel = image.pixel(x, y);
      ASSERT_EQ(qRed(pixel), 42);
    }
  }
}

TEST_F(ImageWrapperTest, GenerateQImage_BGR_Solid) {
  cv::Vec3b orange(0, 165, 255);
  cv::Mat rgbMat(10, 20, CV_8UC3);
  for (int i = 0; i < rgbMat.rows; ++i) {
    for (int j = 0; j < rgbMat.cols; ++j) {
      rgbMat.at<cv::Vec3b>(i, j) = orange;
    }
  }

  ImageWrapper imageWrapper(rgbMat);
  QImage image = imageWrapper.generateQImage();

  EXPECT_EQ(image.width(), rgbMat.cols);
  EXPECT_EQ(image.height(), rgbMat.rows);

  for (int y = 0; y < image.height(); ++y) {
    for (int x = 0; x < image.width(); ++x) {
      QRgb pixel = image.pixel(x, y);
      ASSERT_EQ(qRed(pixel), orange[2]);
      ASSERT_EQ(qGreen(pixel), orange[1]);
      ASSERT_EQ(qBlue(pixel), orange[0]);
    }
  }
}

TEST_F(ImageWrapperTest, GenerateQImage_BGR_Gradient) {
  cv::Mat rgbMat(10, 20, CV_8UC3);
  for (uchar y = 0; y < rgbMat.rows; ++y) {
    for (uchar x = 0; x < rgbMat.cols; ++x) {
      rgbMat.at<cv::Vec3b>(y, x) = cv::Vec3b(y, x, y + x);
    }
  }

  ImageWrapper imageWrapper(rgbMat);
  QImage image = imageWrapper.generateQImage();

  EXPECT_EQ(image.width(), rgbMat.cols);
  EXPECT_EQ(image.height(), rgbMat.rows);

  for (uchar y = 0; y < image.height(); ++y) {
    for (uchar x = 0; x < image.width(); ++x) {
      QRgb pixel = image.pixel(x, y);
      ASSERT_EQ(qBlue(pixel), y);
      ASSERT_EQ(qGreen(pixel), x);
      ASSERT_EQ(qRed(pixel), y + x);
    }
  }
}

TEST_F(ImageWrapperTest, GenerateQImage_BGR_Gradient_BIG) {
  cv::Mat rgbMat(4096, 4096, CV_8UC3);
  for (int y = 0; y < rgbMat.rows; ++y) {
    for (int x = 0; x < rgbMat.cols; ++x) {
      rgbMat.at<cv::Vec3b>(y, x) = cv::Vec3b(y % 255, x % 255, (y + x) % 255);
    }
  }

  ImageWrapper imageWrapper(rgbMat);
  QImage image = imageWrapper.generateQImage();

  ASSERT_FALSE(image.isNull());
  ASSERT_EQ(image.width(), rgbMat.cols);
  ASSERT_EQ(image.height(), rgbMat.rows);


  for (int y = 0; y < image.height(); ++y) {
    for (int x = 0; x < image.width(); ++x) {
      QRgb pixel = image.pixel(x, y);
      ASSERT_EQ(qBlue(pixel), y % 255);
      ASSERT_EQ(qGreen(pixel), x % 255);
      ASSERT_EQ(qRed(pixel), (y + x) % 255);
    }
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
