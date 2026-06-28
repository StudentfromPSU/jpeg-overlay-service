#include "image_processor/ImageValidator.h"
#include "image_processor/Exceptions.h"

#include <opencv2/imgcodecs.hpp>
#include <filesystem>

namespace ImageProcessor
{

    void ImageValidator::ValidateImageFile(const std::string& path) const
    {
        if (path.empty())
        {
            throw ImageException("Path is empty");
        }

        if (!std::filesystem::is_regular_file(path))
        {
            throw ImageException("File does not exist: " + path);
        }

        cv::Mat img = cv::imread(path, cv::IMREAD_UNCHANGED);

        if (img.empty())
        {
            throw ImageException("File is not a valid image: " + path);
        }
    }

    void ImageValidator::ValidateDecodedImage(const cv::Mat& image) const
    {
        if (image.empty())
        {
            throw ImageException("Decoded image is empty");
        }

        if (image.cols <= 0 || image.rows <= 0)
        {
            throw ImageException("Invalid image dimensions");
        }
    }

}