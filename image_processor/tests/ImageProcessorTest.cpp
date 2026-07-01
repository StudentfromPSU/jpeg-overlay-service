#include <gtest/gtest.h>
#include "image_processor/ImageProcessor.h"
#include <filesystem>
#include <opencv2/imgcodecs.hpp>
#include <fstream>

namespace fs = std::filesystem;

class ImageProcessorTest : public ::testing::Test
{
protected:
    ImageProcessor::ImageProcessor processor;
    std::string testImageJpg = "tests/test_images/image.jpg";
    std::string testImagePng = "tests/test_images/image.png";
    std::string outputDir = "tests/output";

    std::vector<uchar> LoadImageFile(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file) return {};
        return std::vector<uchar>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }

    bool SaveImageFile(const std::string& path, const std::vector<uchar>& data)
    {
        std::ofstream file(path, std::ios::binary);
        if (!file) return false;
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        return file.good();
    }

    void SetUp() override
    {
        if (!fs::exists(outputDir))
        {
            fs::create_directories(outputDir);
        }
    }
};

// ProcessValidImage: Successfully process a valid image
TEST_F(ImageProcessorTest, ProcessValidImage)
{
    auto imageData = LoadImageFile(testImageJpg);
    ASSERT_FALSE(imageData.empty()) << "Failed to load test image";

    std::vector<uchar> result;
    auto ec = processor.Process(imageData, "Hello World", result);

    EXPECT_FALSE(ec) << "Processing should succeed with valid image and text";
    EXPECT_FALSE(result.empty()) << "Result should contain encoded image data";
}

// ProcessEmptyCaption: Processing with empty text returns EmptyCaption error
TEST_F(ImageProcessorTest, ProcessEmptyCaption)
{
    auto imageData = LoadImageFile(testImageJpg);
    ASSERT_FALSE(imageData.empty()) << "Failed to load test image";

    std::vector<uchar> result;
    auto ec = processor.Process(imageData, "", result);

    EXPECT_EQ(ec, ImageProcessor::ImageError::EmptyCaption);
}

// ProcessDecodeFailed: Invalid image data returns DecodeFailed error
TEST_F(ImageProcessorTest, ProcessDecodeFailed)
{
    std::vector<uchar> invalidData = {'i', 'n', 'v', 'a', 'l', 'i', 'd'};
    std::vector<uchar> result;
    auto ec = processor.Process(invalidData, "Text", result);

    EXPECT_EQ(ec, ImageProcessor::ImageError::DecodeFailed);
}

// ProcessErrorRecovery: Processor continues working after error
TEST_F(ImageProcessorTest, ProcessErrorRecovery)
{
    std::vector<uchar> invalidData = {'i', 'n', 'v', 'a', 'l', 'i', 'd'};
    std::vector<uchar> result1;
    auto ec1 = processor.Process(invalidData, "text", result1);
    EXPECT_EQ(ec1, ImageProcessor::ImageError::DecodeFailed);

    auto imageData = LoadImageFile(testImageJpg);
    ASSERT_FALSE(imageData.empty()) << "Failed to load test image";

    std::vector<uchar> result2;
    auto ec2 = processor.Process(imageData, "Recovery", result2);
    EXPECT_FALSE(ec2) << "Processing should succeed after previous error";
    EXPECT_FALSE(result2.empty()) << "Result should contain encoded image data";
}

// SaveProcessedImage: Save processed image to file
TEST_F(ImageProcessorTest, SaveProcessedImage)
{
    auto imageData = LoadImageFile(testImageJpg);
    ASSERT_FALSE(imageData.empty()) << "Failed to load test image";

    std::vector<uchar> result;
    auto ec = processor.Process(imageData, "Test Caption", result);
    ASSERT_FALSE(ec) << "Processing should succeed";
    ASSERT_FALSE(result.empty()) << "Result should contain data";

    std::string outputPath = outputDir + "/processed_image.jpg";
    bool saved = SaveImageFile(outputPath, result);
    EXPECT_TRUE(saved) << "Failed to save image to file";
    EXPECT_TRUE(fs::exists(outputPath)) << "Output file should exist";
    EXPECT_GT(fs::file_size(outputPath), 0) << "Output file should not be empty";
}

// SaveProcessedImageAndVerify: Save and verify the image can be loaded
TEST_F(ImageProcessorTest, SaveProcessedImageAndVerify)
{
    auto imageData = LoadImageFile(testImageJpg);
    ASSERT_FALSE(imageData.empty()) << "Failed to load test image";

    std::vector<uchar> result;
    auto ec = processor.Process(imageData, "Verification Test", result);
    ASSERT_FALSE(ec) << "Processing should succeed";
    ASSERT_FALSE(result.empty()) << "Result should contain data";

    std::string outputPath = outputDir + "/verified_image.jpg";
    bool saved = SaveImageFile(outputPath, result);
    ASSERT_TRUE(saved) << "Failed to save image";
    ASSERT_TRUE(fs::exists(outputPath)) << "Output file should exist";

    // Verify saved image can be loaded
    cv::Mat loadedImage = cv::imread(outputPath);
    EXPECT_FALSE(loadedImage.empty()) << "Saved image should be loadable";
    EXPECT_GT(loadedImage.cols, 0) << "Image width should be positive";
    EXPECT_GT(loadedImage.rows, 0) << "Image height should be positive";
}

// MultipleProcessing: Process multiple images with different captions
TEST_F(ImageProcessorTest, MultipleProcessing)
{
    auto imageData = LoadImageFile(testImageJpg);
    ASSERT_FALSE(imageData.empty()) << "Failed to load test image";

    std::vector<std::string> captions = {"First", "Second", "Third"};

    for (size_t i = 0; i < captions.size(); ++i)
    {
        std::vector<uchar> result;
        auto ec = processor.Process(imageData, captions[i], result);
        EXPECT_FALSE(ec) << "Processing should succeed for caption: " << captions[i];
        EXPECT_FALSE(result.empty()) << "Result should contain data for caption: " << captions[i];

        std::string outputPath = outputDir + "/image_" + std::to_string(i + 1) + ".jpg";
        bool saved = SaveImageFile(outputPath, result);
        EXPECT_TRUE(saved) << "Failed to save image " << i + 1;
        EXPECT_TRUE(fs::exists(outputPath)) << "Output file " << i + 1 << " should exist";
    }
}
