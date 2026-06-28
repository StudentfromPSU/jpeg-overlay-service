#pragma once

#include <opencv2/core.hpp>
#include <string>

namespace ImageProcessor 
{

    class CaptionDrawer 
    {
        public:
            cv::Mat AddCaption(const cv::Mat& image, const std::string& text);
        };

}
