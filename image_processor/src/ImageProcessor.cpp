#include "image_processor/ImageProcessor.h"
#include "image_processor/ErrorCode.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <algorithm>
#include <vector>
#include <system_error>

namespace {

    std::error_code DecodeImage(const std::vector<uchar>& imageBytes, cv::Mat& result)
    {
        result = cv::imdecode(imageBytes, cv::IMREAD_UNCHANGED);
        if (result.empty())
        {
            return ImageProcessor::make_error_code(ImageProcessor::ImageError::DecodeFailed);
        }
        return {};
    }

    std::error_code EncodeImage(const cv::Mat& image, std::vector<uchar>& buffer)
    {
        if (!cv::imencode(".jpg", image, buffer) || buffer.empty())
        {
            return ImageProcessor::make_error_code(ImageProcessor::ImageError::EncodeFailed);
        }
        return {};
    }

    std::error_code ValidateCaptionText(const std::string& text)
    {
        if (text.empty())
        {
            return ImageProcessor::make_error_code(ImageProcessor::ImageError::EmptyCaption);
        }
        return {};
    }

    void AddCaption(cv::Mat& image, const std::string& text)
    {
        int fontFace = cv::FONT_HERSHEY_SIMPLEX;
        double fontScale = 1.0;
        int thickness = 2;
        const int padding = 10;

        int baseline = 0;
        cv::Size textSize = cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);

        int x = (image.cols - textSize.width) / 2;
        int y = image.rows - padding - baseline;

        x = std::clamp(x, padding, std::max(padding, image.cols - textSize.width - padding));

        cv::putText(
            image,
            text,
            cv::Point(x, y),
            fontFace,
            fontScale,
            cv::Scalar(0, 0, 0),
            thickness + 2,
            cv::LINE_AA
        );

        cv::putText(
            image,
            text,
            cv::Point(x, y),
            fontFace,
            fontScale,
            cv::Scalar(255, 255, 255),
            thickness,
            cv::LINE_AA
        );
    }

}

namespace ImageProcessor
{

    class ImageProcessor::Impl
    {
    public:
        std::error_code Process(
            const std::vector<uchar>& imageBytes,
            const std::string& text,
            std::vector<uchar>& result)
        {
            if (auto ec = ValidateCaptionText(text))
                return ec;

            cv::Mat image;
            if (auto ec = DecodeImage(imageBytes, image))
                return ec;

            AddCaption(image, text);

            if (auto ec = EncodeImage(image, result))
                return ec;

            return {};
        }
    };

    ImageProcessor::ImageProcessor() : impl(std::make_unique<Impl>()) {}

    ImageProcessor::~ImageProcessor() = default;

    std::error_code ImageProcessor::Process(const std::vector<uchar>& imageBytes, const std::string& text, std::vector<uchar>& result)
    {
        return impl->Process(imageBytes, text, result);
    }

}