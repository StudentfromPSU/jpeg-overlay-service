#include <gtest/gtest.h>
#include "image_processor/Exceptions.h"
#include "image_processor/ImageProcessor.h"
#include <filesystem>

TEST(ImageProcessorFacade, ProcessReturnsImage)
{
    ImageProcessor::ImageProcessor processor;

    cv::Mat result = processor.Process(
        "tests/test_images/image.jpg",
        "Test"
    );

    EXPECT_FALSE(result.empty());
}

TEST(ImageProcessorFacade, SaveWritesFile)
{
    ImageProcessor::ImageProcessor processor;

    cv::Mat img = processor.Process(
        "tests/test_images/image.jpg",
        "Hello"
    );

    std::string out = "tests/test_images/output/final.jpg";

    processor.Save(img, out, 90);

    EXPECT_TRUE(std::filesystem::exists(out));
}

TEST(ImageProcessorFacade, ProcessInvalidFileThrows)
{
    ImageProcessor::ImageProcessor processor;

    EXPECT_THROW(
        processor.Process("unknown.jpg", "text"),
        ImageProcessor::ImageException
    );
}

TEST(ImageProcessorFacade, SaveEmptyImageThrows)
{
    ImageProcessor::ImageProcessor processor;

    cv::Mat empty;

    EXPECT_THROW(
        processor.Save(empty, "out.jpg", 90),
        ImageProcessor::ImageException
    );
}