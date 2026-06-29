#include <gtest/gtest.h>
#include "image_processor/ImageProcessor.h"
#include "image_processor/Exceptions.h"
#include <filesystem>
#include <opencv2/imgcodecs.hpp>

namespace fs = std::filesystem;

class ImageProcessorTest : public ::testing::Test
{
protected:
    ImageProcessor::ImageProcessor processor;
    std::string testImageJpg = "tests/test_images/image.jpg";
    std::string testImagePng = "tests/test_images/image.png";
    std::string outputDir = "tests/output";

    void SetUp() override
    {
        if (!fs::exists(outputDir))
        {
            fs::create_directories(outputDir);
        }
    }
};

TEST_F(ImageProcessorTest, ProcessValidImage)
{
    cv::Mat result = processor.Process(testImageJpg, "Hello World");

    EXPECT_FALSE(result.empty());
    EXPECT_GT(result.cols, 0);
    EXPECT_GT(result.rows, 0);
    EXPECT_EQ(result.channels(), 3);
}

TEST_F(ImageProcessorTest, ProcessInvalidFile)
{
    EXPECT_THROW(
        processor.Process("tests/test_images/nonexistent.jpg", "text"),
        ImageProcessor::ImageException
    );
}

TEST_F(ImageProcessorTest, ProcessInvalidImage)
{
    EXPECT_THROW(
        processor.Process("tests/test_images/text.txt", "caption"),
        ImageProcessor::ImageException
    );
}

TEST_F(ImageProcessorTest, ProcessEmptyCaption)
{
    EXPECT_THROW(
        processor.Process(testImageJpg, ""),
        ImageProcessor::ImageException
    );
}

TEST_F(ImageProcessorTest, ProcessChangesImage)
{
    cv::Mat original = cv::imread(testImageJpg);
    cv::Mat processed = processor.Process(testImageJpg, "Caption");

    EXPECT_FALSE(processed.empty());
    EXPECT_EQ(processed.rows, original.rows);
    EXPECT_EQ(processed.cols, original.cols);
}

TEST_F(ImageProcessorTest, ProcessPreservesDimensions)
{
    cv::Mat original = cv::imread(testImageJpg);
    cv::Mat processed = processor.Process(testImageJpg, "Test");

    EXPECT_EQ(processed.cols, original.cols);
    EXPECT_EQ(processed.rows, original.rows);
}

TEST_F(ImageProcessorTest, SaveValidImage)
{
    cv::Mat img = processor.Process(testImageJpg, "Save Test");
    std::string outputPath = outputDir + "/saved_image.jpg";

    processor.Save(img, outputPath, 95);

    EXPECT_TRUE(fs::exists(outputPath));
    EXPECT_GT(fs::file_size(outputPath), 0);
}

TEST_F(ImageProcessorTest, SaveEmptyImage)
{
    cv::Mat emptyImg;
    std::string outputPath = outputDir + "/empty.jpg";

    EXPECT_THROW(
        processor.Save(emptyImg, outputPath, 90),
        ImageProcessor::ImageException
    );
}

TEST_F(ImageProcessorTest, SaveCreatesReadableImage)
{
    cv::Mat original = processor.Process(testImageJpg, "Readable Test");
    std::string outputPath = outputDir + "/readable_image.jpg";

    processor.Save(original, outputPath, 95);

    cv::Mat loaded = cv::imread(outputPath);
    EXPECT_FALSE(loaded.empty());
    EXPECT_EQ(loaded.cols, original.cols);
    EXPECT_EQ(loaded.rows, original.rows);
}

TEST_F(ImageProcessorTest, SaveInvalidPath)
{
    cv::Mat img = processor.Process(testImageJpg, "Invalid Path");
    std::string invalidPath = outputDir + "/invalid_nested/deep/folder/image.jpg";

    try
    {
        processor.Save(img, invalidPath, 95);
    }
    catch (const ImageProcessor::ImageException&)
    {
        // Ожидаемое исключение
    }
}

TEST_F(ImageProcessorTest, FullWorkflow)
{
    std::string outputPath = outputDir + "/full_workflow.jpg";

    cv::Mat processed = processor.Process(testImageJpg, "Workflow Workflow Workflow Workflow Workflow Workflow");
    processor.Save(processed, outputPath, 90);

    EXPECT_TRUE(fs::exists(outputPath));

    cv::Mat reloaded = cv::imread(outputPath);
    EXPECT_FALSE(reloaded.empty());
    EXPECT_EQ(reloaded.rows, processed.rows);
    EXPECT_EQ(reloaded.cols, processed.cols);
}

TEST_F(ImageProcessorTest, ExceptionRecovery)
{
    try
    {
        processor.Process("nonexistent.jpg", "text");
    }
    catch (const ImageProcessor::ImageException&)
    {
        // Ожидаемое исключение
    }

    cv::Mat result = processor.Process(testImageJpg, "Recovery");
    EXPECT_FALSE(result.empty());

    std::string outputPath = outputDir + "/recovery_test.jpg";
    processor.Save(result, outputPath, 95);
    EXPECT_TRUE(fs::exists(outputPath));
}

