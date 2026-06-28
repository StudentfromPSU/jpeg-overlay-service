#include <gtest/gtest.h>
#include <opencv2/imgcodecs.hpp>
#include "image_processor/ImageValidator.h"
#include "image_processor/Exceptions.h"

using namespace ImageProcessor;

TEST(ImageValidator, EmptyPathThrows)
{
    ImageValidator validator;

    EXPECT_THROW(
        validator.ValidateImageFile(""),
        ImageException
    );
}

TEST(ImageValidator, MissingFileThrows)
{
    ImageValidator validator;

    EXPECT_THROW(
        validator.ValidateImageFile("unknown.jpg"),
        ImageException
    );
}

TEST(ImageValidator, ValidImageFilePasses)
{
    ImageValidator validator;

    EXPECT_NO_THROW(
        validator.ValidateImageFile("tests/test_images/image.jpg")
    );
}

TEST(ImageValidator, EmptyDecodedImageThrows)
{
    ImageValidator validator;
    cv::Mat empty;

    EXPECT_THROW(
        validator.ValidateDecodedImage(empty),
        ImageException
    );
}

TEST(ImageValidator, ValidDecodedImagePasses)
{
    ImageValidator validator;

    cv::Mat img = cv::imread("tests/test_images/image.jpg");

    ASSERT_FALSE(img.empty());

    EXPECT_NO_THROW(
        validator.ValidateDecodedImage(img)
    );
}