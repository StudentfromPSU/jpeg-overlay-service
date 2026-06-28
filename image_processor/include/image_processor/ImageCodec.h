#pragma once

#include <opencv2/core.hpp>
#include <string>

namespace ImageProcessor 
{

    class ImageCodec 
    {

    public:

        static cv::Mat Decode(const std::string& path);
        static void Encode(const cv::Mat& image, const std::string& path, int quality = 95);
    };

}