#include <QImage>
#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include <qpixmap.h>

#include "../src/imageWrapper.hpp"

// Test fixture for ImageWrapper
class ImageWrapperTest : public ::testing::Test {
protected:
  void SetUp() override {}

  void TearDown() override {}
};

// // Test ImageType conversion functions
// TEST_F(ImageWrapperTest, ImageTypeConversionTest) {
//     // Test imageTypeFromQImageFormat
//     EXPECT_EQ(imageTypeFromQImageFormat(QImage::Format_Mono),
//     ImageType::Binary);
//     EXPECT_EQ(imageTypeFromQImageFormat(QImage::Format_Grayscale8),
//     ImageType::GrayScale);
//     EXPECT_EQ(imageTypeFromQImageFormat(QImage::Format_RGB888),
//     ImageType::RGB);
//     EXPECT_EQ(imageTypeFromQImageFormat(QImage::Format_ARGB32),
//     std::nullopt);

//     // Test toString
//     EXPECT_STREQ(toString(ImageType::Binary), "binary");
//     EXPECT_STREQ(toString(ImageType::GrayScale), "grayscale");
//     EXPECT_STREQ(toString(ImageType::RGB), "RGB");

//     // Test qimageFormat
//     EXPECT_EQ(qimageFormat(ImageType::Binary), QImage::Format_Mono);
//     EXPECT_EQ(qimageFormat(ImageType::GrayScale), QImage::Format_Grayscale8);
//     EXPECT_EQ(qimageFormat(ImageType::HSV), std::nullopt);

//     // Test cvImageType
//     EXPECT_EQ(cvImageType(ImageType::Binary), CV_8UC1);
//     EXPECT_EQ(cvImageType(ImageType::RGB), CV_8UC3);
// }

// test ImageWrapper constructor from cv::Mat
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

// helper function to create an OpenCV Mat with a specific type and BGR color
cv::Mat createMat(int width, int height, cv::Vec3b color) {
  cv::Mat mat(height, width, CV_8UC3);
  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      mat.at<cv::Vec3b>(i, j) = color;
    }
  }
  return mat;
}

TEST_F(ImageWrapperTest, GenerateQImage_BGR) {
  // OpenCV by default stores as BGR
  cv::Vec3b orange(0, 165, 255);
  cv::Mat rgbMat = createMat(10, 20, orange);
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

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
