#include <gtest/gtest.h>
#include "image_processor/CaptionDrawer.h"
#include "image_processor/Exceptions.h"
#include <opencv2/imgcodecs.hpp>

using namespace ImageProcessor;

TEST(CaptionDrawer, EmptyImageThrows)
{
    CaptionDrawer drawer;
    cv::Mat empty;

    EXPECT_THROW(
        drawer.AddCaption(empty, "text"),
        ImageException
    );
}

TEST(CaptionDrawer, EmptyTextThrows)
{
    CaptionDrawer drawer;

    cv::Mat img = cv::imread("tests/test_images/image.jpg");

    EXPECT_THROW(
        drawer.AddCaption(img, ""),
        ImageException
    );
}

TEST(CaptionDrawer, OutputDiffersFromInput)
{
    CaptionDrawer drawer;

    cv::Mat img = cv::imread("tests/test_images/image.jpg");

    cv::Mat result = drawer.AddCaption(img, "Hello");

    EXPECT_NE(cv::sum(img)[0], cv::sum(result)[0]);
}