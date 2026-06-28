#include <gtest/gtest.h>
#include "image_processor/ImageCodec.h"
#include "image_processor/Exceptions.h"
#include <filesystem>

using namespace ImageProcessor;

TEST(ImageCodec, DecodeValidImage)
{
    cv::Mat img = ImageCodec::Decode("tests/test_images/image.jpg");

    EXPECT_FALSE(img.empty());
}

TEST(ImageCodec, DecodeInvalidImage)
{
    cv::Mat img = ImageCodec::Decode("unknown.jpg");

    EXPECT_TRUE(img.empty());
}

TEST(ImageCodec, EncodeCreatesFile)
{
    cv::Mat img = ImageCodec::Decode("tests/test_images/image.jpg");

    std::string out = "tests/test_images/output/codec_test.jpg";

    ImageCodec::Encode(img, out, 90);

    EXPECT_TRUE(std::filesystem::exists(out));
}