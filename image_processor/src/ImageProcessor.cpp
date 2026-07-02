#include "image_processor/ImageProcessor.h"
#include "image_processor/Exceptions.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <algorithm>

namespace {

    cv::Mat DecodeImage(const std::vector<ImageProcessor::Byte>& imageBytes)
    {
        return cv::imdecode(imageBytes, cv::IMREAD_UNCHANGED);
    }

    std::vector<ImageProcessor::Byte> EncodeImage(const cv::Mat& image)
    {
        std::vector<ImageProcessor::Byte> buffer;
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

    void ValidateCaptionText(const std::string& text)
    {
        if (text.empty())
        {
            throw ImageProcessor::ImageException("Caption text is empty");
        }
    }

    void ValidateImageBytes(const std::vector<ImageProcessor::Byte>& imageBytes)
    {
        if (imageBytes.empty())
        {
            throw ImageProcessor::ImageException("Image data is empty");
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
        std::vector<Byte> Process(const std::vector<Byte>& imageBytes, const std::string& text)
        {
            ValidateImageBytes(imageBytes);

            cv::Mat image = DecodeImage(imageBytes);

            ValidateDecodedImage(image);

            ValidateCaptionText(text);

            AddCaption(image, text);

            return EncodeImage(image);
        }
    };

    ImageProcessor::ImageProcessor() : impl(std::make_unique<Impl>()) {}

    ImageProcessor::~ImageProcessor() = default;

    std::vector<Byte> ImageProcessor::Process(const std::vector<Byte>& imageBytes, const std::string& text)
    {
        return impl->Process(imageBytes, text);
    }

}