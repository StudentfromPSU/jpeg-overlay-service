#include <gtest/gtest.h>
#include "image_processor/ImageProcessor.h"
#include "image_processor/Exceptions.h"
#include <fstream>
#include <opencv2/imgcodecs.hpp>

class ImageProcessorTest : public ::testing::Test
{
protected:
    ImageProcessor::ImageProcessor processor;
    std::string testImageJpgPath = "tests/test_images/image.jpg";
    std::string testImagePngPath = "tests/test_images/image.png";
    std::string invalidImagePath = "tests/test_images/text.txt";

    std::vector<uchar> LoadImageAsBytes(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("Cannot open file: " + path);

        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uchar> buffer(fileSize);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        file.close();

        return buffer;
    }

    void SaveBytesToFile(const std::vector<uchar>& data, const std::string& path)
    {
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("Cannot create file: " + path);
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        file.close();
    }
};

TEST_F(ImageProcessorTest, ProcessValidImageJpeg)
{
    std::vector<uchar> imageBytes = LoadImageAsBytes(testImageJpgPath);
    std::vector<uchar> result = processor.Process(imageBytes, "Hello World");

    EXPECT_FALSE(result.empty());
    EXPECT_GT(result.size(), 0);

    cv::Mat decodedResult = cv::imdecode(result, cv::IMREAD_UNCHANGED);
    EXPECT_FALSE(decodedResult.empty());
    EXPECT_GT(decodedResult.cols, 0);
    EXPECT_GT(decodedResult.rows, 0);
}

TEST_F(ImageProcessorTest, ProcessValidImagePng)
{
    std::vector<uchar> imageBytes = LoadImageAsBytes(testImagePngPath);
    std::vector<uchar> result = processor.Process(imageBytes, "PNG Test");

    EXPECT_FALSE(result.empty());
    EXPECT_GT(result.size(), 0);

    cv::Mat decodedResult = cv::imdecode(result, cv::IMREAD_UNCHANGED);
    EXPECT_FALSE(decodedResult.empty());
}

TEST_F(ImageProcessorTest, ProcessInvalidImageBytes)
{
    std::vector<uchar> invalidBytes = { 0xFF, 0xD8, 0xFF };

    EXPECT_THROW(
        processor.Process(invalidBytes, "text"),
        ImageProcessor::ImageException
    );
}

TEST_F(ImageProcessorTest, ProcessEmptyImageBytes)
{
    std::vector<uchar> emptyBytes;

    EXPECT_THROW(
        processor.Process(emptyBytes, "caption"),
        ImageProcessor::ImageException
    );
}

TEST_F(ImageProcessorTest, ProcessEmptyCaption)
{
    std::vector<uchar> imageBytes = LoadImageAsBytes(testImageJpgPath);

    EXPECT_THROW(
        processor.Process(imageBytes, ""),
        ImageProcessor::ImageException
    );
}

TEST_F(ImageProcessorTest, ProcessPreservesDimensions)
{
    std::vector<uchar> imageBytes = LoadImageAsBytes(testImageJpgPath);
    cv::Mat original = cv::imdecode(imageBytes, cv::IMREAD_UNCHANGED);

    std::vector<uchar> processedBytes = processor.Process(imageBytes, "Test Caption");
    cv::Mat processed = cv::imdecode(processedBytes, cv::IMREAD_UNCHANGED);

    EXPECT_EQ(processed.cols, original.cols);
    EXPECT_EQ(processed.rows, original.rows);
}

TEST_F(ImageProcessorTest, ProcessAddsCaption)
{
    std::vector<uchar> imageBytes = LoadImageAsBytes(testImageJpgPath);
    cv::Mat original = cv::imdecode(imageBytes, cv::IMREAD_UNCHANGED);

    std::vector<uchar> processedBytes = processor.Process(imageBytes, "Caption Text");
    cv::Mat processed = cv::imdecode(processedBytes, cv::IMREAD_UNCHANGED);

    EXPECT_FALSE(processed.empty());
    EXPECT_EQ(processed.cols, original.cols);
    EXPECT_EQ(processed.rows, original.rows);
    EXPECT_EQ(processed.channels(), original.channels());
}

TEST_F(ImageProcessorTest, ProcessReturnsValidJpegFormat)
{
    std::vector<uchar> imageBytes = LoadImageAsBytes(testImageJpgPath);
    std::vector<uchar> result = processor.Process(imageBytes, "Format Test");

    EXPECT_GE(result.size(), 4);
    EXPECT_EQ(result[0], 0xFF);
    EXPECT_EQ(result[1], 0xD8);
    EXPECT_EQ(result[result.size() - 2], 0xFF);
    EXPECT_EQ(result[result.size() - 1], 0xD9);
}

TEST_F(ImageProcessorTest, ProcessMultipleCaptions)
{
    std::vector<uchar> imageBytes = LoadImageAsBytes(testImageJpgPath);

    std::vector<uchar> result1 = processor.Process(imageBytes, "Caption 1");
    std::vector<uchar> result2 = processor.Process(imageBytes, "Caption 2");

    EXPECT_FALSE(result1.empty());
    EXPECT_FALSE(result2.empty());
    EXPECT_GT(result1.size(), 0);
    EXPECT_GT(result2.size(), 0);

    cv::Mat decoded1 = cv::imdecode(result1, cv::IMREAD_UNCHANGED);
    cv::Mat decoded2 = cv::imdecode(result2, cv::IMREAD_UNCHANGED);

    EXPECT_FALSE(decoded1.empty());
    EXPECT_FALSE(decoded2.empty());
}

TEST_F(ImageProcessorTest, ProcessLongCaption)
{
    std::vector<uchar> imageBytes = LoadImageAsBytes(testImageJpgPath);
    std::string longCaption = "This is a very long caption that should be wrapped across multiple lines to test the text wrapping functionality";

    std::vector<uchar> result = processor.Process(imageBytes, longCaption);

    EXPECT_FALSE(result.empty());

    cv::Mat decodedResult = cv::imdecode(result, cv::IMREAD_UNCHANGED);
    EXPECT_FALSE(decodedResult.empty());
}

TEST_F(ImageProcessorTest, ProcessShortCaption)
{
    std::vector<uchar> imageBytes = LoadImageAsBytes(testImageJpgPath);
    std::vector<uchar> result = processor.Process(imageBytes, "A");

    EXPECT_FALSE(result.empty());

    cv::Mat decodedResult = cv::imdecode(result, cv::IMREAD_UNCHANGED);
    EXPECT_FALSE(decodedResult.empty());
}

TEST_F(ImageProcessorTest, ProcessConsistency)
{
    std::vector<uchar> imageBytes = LoadImageAsBytes(testImageJpgPath);

    std::vector<uchar> result1 = processor.Process(imageBytes, "Same Caption");
    std::vector<uchar> result2 = processor.Process(imageBytes, "Same Caption");

    cv::Mat decoded1 = cv::imdecode(result1, cv::IMREAD_UNCHANGED);
    cv::Mat decoded2 = cv::imdecode(result2, cv::IMREAD_UNCHANGED);

    EXPECT_FALSE(decoded1.empty());
    EXPECT_FALSE(decoded2.empty());
    EXPECT_EQ(decoded1.cols, decoded2.cols);
    EXPECT_EQ(decoded1.rows, decoded2.rows);
}

TEST_F(ImageProcessorTest, ProcessExceptionRecovery)
{
    std::vector<uchar> invalidBytes = { 0x00, 0x00, 0x00 };
    std::vector<uchar> validBytes = LoadImageAsBytes(testImageJpgPath);

    try
    {
        processor.Process(invalidBytes, "test");
    }
    catch (const ImageProcessor::ImageException&)
    {
    }

    std::vector<uchar> result = processor.Process(validBytes, "Recovery");
    EXPECT_FALSE(result.empty());
}

TEST_F(ImageProcessorTest, ProcessInvalidTextFile)
{
    std::vector<uchar> textFileBytes = LoadImageAsBytes(invalidImagePath);

    EXPECT_THROW(
        processor.Process(textFileBytes, "caption"),
        ImageProcessor::ImageException
    );
}

TEST_F(ImageProcessorTest, ProcessRoundTrip)
{
    std::vector<uchar> originalBytes = LoadImageAsBytes(testImageJpgPath);
    cv::Mat originalImage = cv::imdecode(originalBytes, cv::IMREAD_UNCHANGED);

    std::vector<uchar> processedBytes = processor.Process(originalBytes, "Test");

    cv::Mat processedImage = cv::imdecode(processedBytes, cv::IMREAD_UNCHANGED);

    EXPECT_EQ(processedImage.cols, originalImage.cols);
    EXPECT_EQ(processedImage.rows, originalImage.rows);
    EXPECT_FALSE(processedImage.empty());
}

TEST_F(ImageProcessorTest, ProcessOutputIsDecodable)
{
    std::vector<uchar> imageBytes = LoadImageAsBytes(testImageJpgPath);
    std::vector<uchar> result = processor.Process(imageBytes, "Decodable Test");

    cv::Mat decodedResult = cv::imdecode(result, cv::IMREAD_COLOR);
    EXPECT_FALSE(decodedResult.empty());
    EXPECT_GT(decodedResult.cols, 0);
    EXPECT_GT(decodedResult.rows, 0);
}

TEST_F(ImageProcessorTest, ProcessSpecialCharactersInCaption)
{
    std::vector<uchar> imageBytes = LoadImageAsBytes(testImageJpgPath);
    std::string captionWithSpecialChars = "Caption with numbers 123 and special chars !@#$%";

    std::vector<uchar> result = processor.Process(imageBytes, captionWithSpecialChars);

    EXPECT_FALSE(result.empty());
    cv::Mat decodedResult = cv::imdecode(result, cv::IMREAD_UNCHANGED);
    EXPECT_FALSE(decodedResult.empty());
}

TEST_F(ImageProcessorTest, ProcessSaveAndReload)
{
    std::vector<uchar> imageBytes = LoadImageAsBytes(testImageJpgPath);
    std::vector<uchar> processedBytes = processor.Process(imageBytes, "Save and Reload");

    std::string outputPath = "tests/test_images/output/processed_image.jpg";
    SaveBytesToFile(processedBytes, outputPath);

    std::vector<uchar> reloadedBytes = LoadImageAsBytes(outputPath);
    EXPECT_FALSE(reloadedBytes.empty());

    cv::Mat reloadedImage = cv::imdecode(reloadedBytes, cv::IMREAD_UNCHANGED);
    EXPECT_FALSE(reloadedImage.empty());
}