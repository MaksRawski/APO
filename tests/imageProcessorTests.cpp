#include <QImage>
#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>

#include "../src/imageProcessor.hpp"
#include "../src/imageWrapper.hpp"

using imageProcessor::LUT;

class ImageProcessorTest : public ::testing::Test {
protected:
  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(ImageProcessorTest, HistogramTest) {
  cv::Mat testMat(1, 10, CV_8UC1);
  uchar testData[] = {0, 1, 1, 2, 2, 2, 3, 3, 3, 3};
  memcpy(testMat.data, testData, sizeof(testData));
  ImageWrapper image(testMat);

  LUT histogram_result = imageProcessor::histogram(image);

  LUT expected_histogram(256, 0);
  expected_histogram[0] = 1;
  expected_histogram[1] = 2;
  expected_histogram[2] = 3;
  expected_histogram[3] = 4;
  EXPECT_EQ(histogram_result, expected_histogram);
}

TEST_F(ImageProcessorTest, ApplyNegateLUT) {
  cv::Mat testMat(1, 10, CV_8UC1);
  uchar testData[] = {0, 42, 55, 200, 255};
  memcpy(testMat.data, testData, sizeof(testData));
  ImageWrapper image(testMat);

  LUT negateLut = imageProcessor::negate();
  ImageWrapper negation = imageProcessor::applyLUT(image, negateLut);

  cv::Mat negationMat = negation.getMat();
  ASSERT_EQ(negationMat.at<uchar>(0, 0), 255 - testData[0]);
  ASSERT_EQ(negationMat.at<uchar>(0, 1), 255 - testData[1]);
  ASSERT_EQ(negationMat.at<uchar>(0, 2), 255 - testData[2]);
  ASSERT_EQ(negationMat.at<uchar>(0, 3), 255 - testData[3]);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
