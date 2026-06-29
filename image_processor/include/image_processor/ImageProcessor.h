#pragma once

#include <memory>
#include <filesystem>
#include <string>
#include <opencv2/core.hpp>

namespace ImageProcessor
{
    class ImageProcessor
    {
    public:
        ImageProcessor();
        ~ImageProcessor();

        cv::Mat Process(const std::filesystem::path& input, const std::string& text);
        void Save(const cv::Mat& image, const std::filesystem::path& output, int quality);

    private:
        class Impl;
        std::unique_ptr<Impl> impl;
    };

}
