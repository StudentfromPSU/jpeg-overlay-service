#pragma once

#include <string>
#include <opencv2/core.hpp>

namespace ImageProcessor
{

    class ImageValidator
    {
    public:

        void ValidateImageFile(const std::string& path) const;

        void ValidateDecodedImage(const cv::Mat& image) const;
    };

}