#pragma once

#include <memory>
#include <vector>
#include <string>
#include <opencv2/core.hpp>

namespace ImageProcessor
{
    class ImageProcessor
    {
    public:
        ImageProcessor();
        ~ImageProcessor();

        std::vector<uchar> Process(const std::vector<uchar>& imageBytes, const std::string& text);

    private:
        class Impl;
        std::unique_ptr<Impl> impl;
    };

}
