#include <gtest/gtest.h>
#include <QImage>
#include <opencv2/opencv.hpp>

#include "../src/imageWrapper.hpp"
#include "../src/imageProcessor.hpp"

class ImageProcessorTest : public ::testing::Test {
protected:
    // Optional setup method that runs before each test
    void SetUp() override {
        // You can add any common setup code here
    }

    // Optional teardown method that runs after each test
    void TearDown() override {
        // You can add any cleanup code here
    }
};

TEST_F(ImageProcessorTest, HistogramTest) {
    // Prepare test data
    std::vector<uchar> testData = {0, 1, 1, 2, 2, 2, 3, 3, 3, 3};
    int width = 10;
    int height = 1;
    ImageType type = ImageType::GrayScale;

    // Create ImageWrapper
    ImageWrapper image(testData, width, height, type);

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
    std::vector<uchar> emptyData;
    ImageWrapper emptyImage(emptyData, 0, 0, ImageType::GrayScale);

    imageProcessor::LUT empty_histogram = imageProcessor::histogram(emptyImage);
    EXPECT_TRUE(std::all_of(empty_histogram.begin(), empty_histogram.end(),
                             [](int count) { return count == 0; }));

    // Image with single value
    std::vector<uchar> singleValueData(100, 42);
    ImageWrapper singleValueImage(singleValueData, 10, 10, ImageType::GrayScale);

    imageProcessor::LUT single_histogram = imageProcessor::histogram(singleValueImage);
    EXPECT_EQ(single_histogram[42], 100);
    EXPECT_TRUE(std::all_of(single_histogram.begin(), single_histogram.end(),
                             [&](int count) { return count == 0 || count == 100; }));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
