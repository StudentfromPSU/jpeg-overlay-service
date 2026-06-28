#include "image_processor/CaptionDrawer.h"
#include "image_processor/Exceptions.h"

#include <opencv2/imgproc.hpp>
#include <algorithm>

namespace ImageProcessor
{

    cv::Mat CaptionDrawer::AddCaption(const cv::Mat& image, const std::string& text)
    {
        if (image.empty())
        {
            throw ImageException("Empty image provided");
        }

        if (text.empty())
        {
            throw ImageException("Caption text cannot be empty");
        }

        cv::Mat result = image.clone();

        int fontFace = cv::FONT_HERSHEY_SIMPLEX;
        double fontScale = 1.0;
        int thickness = 2;

        int baseline = 0;
        cv::Size textSize = cv::getTextSize(
            text,
            fontFace,
            fontScale,
            thickness,
            &baseline
        );

        const int padding = 10;

        int x = (result.cols - textSize.width) / 2;
        int y = result.rows - padding - baseline;

        x = std::clamp(
            x,
            padding,
            std::max(
                padding,
                result.cols - textSize.width - padding
            )
        );

        if (y - textSize.height < padding)
        {
            y = textSize.height + padding;
        }

        if (y + baseline > result.rows)
        {
            y = result.rows - baseline;
        }

        cv::putText(
            result,
            text,
            cv::Point(x, y),
            fontFace,
            fontScale,
            cv::Scalar(0, 0, 0),
            thickness + 2,
            cv::LINE_AA
        );

        cv::putText(
            result,
            text,
            cv::Point(x, y),
            fontFace,
            fontScale,
            cv::Scalar(255, 255, 255),
            thickness,
            cv::LINE_AA
        );

        return result;
    }

}