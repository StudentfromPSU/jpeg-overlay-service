#include "image_processor/ImageProcessor.h"
#include "image_processor/Exceptions.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <algorithm>
#include <vector>

namespace {

    cv::Mat DecodeImage(const std::vector<uchar>& imageBytes)
    {
        return cv::imdecode(imageBytes, cv::IMREAD_UNCHANGED);
    }

    std::vector<uchar> EncodeImage(const cv::Mat& image)
    {
        std::vector<uchar> buffer;
        cv::imencode(".jpg", image, buffer);
        return buffer;
    }

    void ValidateDecodedImage(const cv::Mat& image)
    {
        if (image.empty())
        {
            throw ImageProcessor::ImageException("Decoded image is empty");
        }
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
        std::vector<uchar> Process(const std::vector<uchar>& imageBytes, const std::string& text)
        {

            cv::Mat image = DecodeImage(imageBytes);

            ValidateDecodedImage(image);

            AddCaption(image, text);

            return EncodeImage(image);
        }
    };

    ImageProcessor::ImageProcessor() : impl(std::make_unique<Impl>()) {}

    ImageProcessor::~ImageProcessor() = default;

    std::vector<uchar> ImageProcessor::Process(const std::vector<uchar>& imageBytes, const std::string& text)
    {
        return impl->Process(imageBytes, text);
    }

}