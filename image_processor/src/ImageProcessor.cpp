#include "image_processor/ImageProcessor.h"
#include "image_processor/Exceptions.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <filesystem>
#include <algorithm>
#include <vector>

namespace {

    cv::Mat DecodeImage(const std::filesystem::path& path)
    {
        cv::Mat img = cv::imread(path.string(), cv::IMREAD_UNCHANGED);
        return img;
    }

    void EncodeImage(const cv::Mat& image, const std::filesystem::path& path, int quality)
    {
        std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, quality};

        cv::imwrite(path.string(), image, params);
    }

    void ValidateImageFile(const std::filesystem::path& path)
    {
        if (!std::filesystem::is_regular_file(path))
        {
            throw ImageProcessor::ImageException("File does not exist: " + path.string());
        }
    }

    void ValidateDecodedImage(const cv::Mat& image)
    {
        if (image.empty())
        {
            throw ImageProcessor::ImageException("Decoded image is empty");
        }

        if (image.cols <= 0 || image.rows <= 0)
        {
            throw ImageProcessor::ImageException("Invalid image dimensions");
        }
    }

    void ValidateCaptionText(const std::string& text)
    {
        if (text.empty())
        {
            throw ImageProcessor::ImageException("Caption text cannot be empty");
        }
    }

    std::vector<std::string> WrapText(const std::string& text, int maxWidth, int fontFace, double fontScale, int thickness)
    {
        std::vector<std::string> lines;
        std::string currentLine;
        std::string word;

        for (size_t i = 0; i <= text.length(); ++i)
        {
            char c = (i < text.length()) ? text[i] : ' ';

            if (c == ' ' || i == text.length())
            {
                if (!word.empty())
                {
                    std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
                    int baseline = 0;
                    cv::Size textSize = cv::getTextSize(testLine, fontFace, fontScale, thickness, &baseline);

                    if (textSize.width > maxWidth && !currentLine.empty())
                    {
                        lines.push_back(currentLine);
                        currentLine = word;
                    }
                    else
                    {
                        currentLine = testLine;
                    }

                    word.clear();
                }
            }
            else
            {
                word += c;
            }
        }

        if (!currentLine.empty())
        {
            lines.push_back(currentLine);
        }

        return lines;
    }

    void AddCaption(cv::Mat& image, const std::string& text)
    {
        int fontFace = cv::FONT_HERSHEY_SIMPLEX;
        double fontScale = 1.0;
        int thickness = 2;
        const int padding = 10;
        const int lineSpacing = 5;

        int baseline = 0;
        cv::Size singleLineSize = cv::getTextSize(
            "A",
            fontFace,
            fontScale,
            thickness,
            &baseline
        );

        int maxWidth = image.cols - 2 * padding;
        std::vector<std::string> lines = WrapText(text, maxWidth, fontFace, fontScale, thickness);

        int totalHeight = lines.size() * (singleLineSize.height + lineSpacing);

        int startY = image.rows - padding - baseline;

        if (startY - totalHeight < padding)
        {
            startY = totalHeight + padding;
        }

        for (size_t i = 0; i < lines.size(); ++i)
        {
            int baseline = 0;
            cv::Size lineSize = cv::getTextSize(
                lines[i],
                fontFace,
                fontScale,
                thickness,
                &baseline
            );

            int x = (image.cols - lineSize.width) / 2;
            int y = startY - (lines.size() - i - 1) * (singleLineSize.height + lineSpacing);

            x = std::clamp(
                x,
                padding,
                std::max(
                    padding,
                    image.cols - lineSize.width - padding
                )
            );

            cv::putText(
                image,
                lines[i],
                cv::Point(x, y),
                fontFace,
                fontScale,
                cv::Scalar(0, 0, 0),
                thickness + 2,
                cv::LINE_AA
            );

            cv::putText(
                image,
                lines[i],
                cv::Point(x, y),
                fontFace,
                fontScale,
                cv::Scalar(255, 255, 255),
                thickness,
                cv::LINE_AA
            );
        }
    }

}

namespace ImageProcessor
{

    class ImageProcessor::Impl
    {
    public:
        cv::Mat Process(const std::filesystem::path& input, const std::string& text)
        {
            ValidateImageFile(input);

            ValidateCaptionText(text);

            cv::Mat image = DecodeImage(input);

            ValidateDecodedImage(image);

            AddCaption(image, text);

            return image;
        }

        void Save(const cv::Mat& image, const std::filesystem::path& output, int quality)
        {
            ValidateDecodedImage(image);

            EncodeImage(image, output, quality);
        }
    };

    ImageProcessor::ImageProcessor() : impl(std::make_unique<Impl>()) {}

    ImageProcessor::~ImageProcessor() = default;

    cv::Mat ImageProcessor::Process(const std::filesystem::path& input, const std::string& text)
    {
        return impl->Process(input, text);
    }

    void ImageProcessor::Save(const cv::Mat& image, const std::filesystem::path& output, int quality)
    {
        impl->Save(image, output, quality);
    }

}
