#include <gtest/gtest.h>
#include <QImage>
#include <opencv2/opencv.hpp>

#include "../src/imageWrapper.hpp"

// Test fixture for ImageWrapper
class ImageWrapperTest : public ::testing::Test {
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

// Test ImageType conversion functions
TEST_F(ImageWrapperTest, ImageTypeConversionTest) {
    // Test imageTypeFromQImageFormat
    EXPECT_EQ(imageTypeFromQImageFormat(QImage::Format_Mono), ImageType::Binary);
    EXPECT_EQ(imageTypeFromQImageFormat(QImage::Format_Grayscale8), ImageType::GrayScale);
    EXPECT_EQ(imageTypeFromQImageFormat(QImage::Format_RGB888), ImageType::RGB);
    EXPECT_EQ(imageTypeFromQImageFormat(QImage::Format_ARGB32), std::nullopt);

    // Test toString
    EXPECT_STREQ(toString(ImageType::Binary), "binary");
    EXPECT_STREQ(toString(ImageType::GrayScale), "grayscale");
    EXPECT_STREQ(toString(ImageType::RGB), "RGB");

    // Test bitsPerPixel
    EXPECT_EQ(bitsPerPixel(ImageType::Binary), 1);
    EXPECT_EQ(bitsPerPixel(ImageType::GrayScale), 8);
    EXPECT_EQ(bitsPerPixel(ImageType::RGB), 3 * 8);

    // Test qimageFormat
    EXPECT_EQ(qimageFormat(ImageType::Binary), QImage::Format_Mono);
    EXPECT_EQ(qimageFormat(ImageType::GrayScale), QImage::Format_Grayscale8);
    EXPECT_EQ(qimageFormat(ImageType::HSV), std::nullopt);

    // Test cvImageType
    EXPECT_EQ(cvImageType(ImageType::Binary), CV_8UC1);
    EXPECT_EQ(cvImageType(ImageType::RGB), CV_8UC3);
}

// Test ImageWrapper constructor and basic getters
TEST_F(ImageWrapperTest, ImageWrapperConstructorTest) {
    // Prepare test data
    std::vector<uchar> testData = {1, 2, 3, 4, 5};
    int width = 5;
    int height = 1;
    ImageType type = ImageType::GrayScale;

    // Create ImageWrapper
    ImageWrapper image(testData, width, height, type);

    // Test getters
    EXPECT_EQ(image.getWidth(), width);
    EXPECT_EQ(image.getHeight(), height);
    EXPECT_EQ(image.getType(), type);

    // Compare raw data
    const auto& rawData = image.getRawData();
    EXPECT_EQ(rawData, testData);
}

// Test histogram function

// Additional tests can be added as needed

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
