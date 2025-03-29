#include <QImage>
#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>

#include "../src/imageProcessor.hpp"
#include "../src/imageWrapper.hpp"

class ImageProcessorTest : public ::testing::Test {
protected:
  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(ImageProcessorTest, HistogramTest) {
  // Prepare test data
  cv::Mat testData{0, 1, 1, 2, 2, 2, 3, 3, 3, 3};

  // Create ImageWrapper
  ImageWrapper image(testData);

  // Compute histogram
  imageProcessor::LUT histogram_result = imageProcessor::histogram(image);

  // Expected histogram
  std::vector<int> expected_histogram(256, 0);
  expected_histogram[0] = 1;
  expected_histogram[1] = 2;
  expected_histogram[2] = 3;
  expected_histogram[3] = 4;

  // Compare histograms
  EXPECT_EQ(histogram_result, expected_histogram);
}

// Test histogram edge cases
TEST_F(ImageProcessorTest, HistogramEdgeCasesTest) {
  // Empty image
  cv::Mat emptyData;
  ImageWrapper emptyImage(emptyData);

  imageProcessor::LUT empty_histogram = imageProcessor::histogram(emptyImage);
  EXPECT_TRUE(std::all_of(empty_histogram.begin(), empty_histogram.end(),
                          [](int count) { return count == 0; }));

  // Image with single value
  cv::Mat singleValueData{1};
  ImageWrapper singleValueImage(singleValueData);

  imageProcessor::LUT single_histogram =
      imageProcessor::histogram(singleValueImage);
  EXPECT_EQ(single_histogram[42], 100);
  EXPECT_TRUE(
      std::all_of(single_histogram.begin(), single_histogram.end(),
                  [&](int count) { return count == 0 || count == 100; }));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
