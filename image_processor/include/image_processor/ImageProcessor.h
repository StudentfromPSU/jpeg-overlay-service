#pragma once
#include "image_processor/ImageCodec.h"
#include "image_processor/ImageValidator.h"
#include "image_processor/CaptionDrawer.h"
#include <string>

namespace ImageProcessor 
{

    class ImageProcessor 
    {
    private:

        ImageValidator validator;
        CaptionDrawer drawer;

    public:

        cv::Mat Process(const std::string& input, const std::string& text);

        void Save(const cv::Mat& image, const std::string& output, int quality);
    };

}
