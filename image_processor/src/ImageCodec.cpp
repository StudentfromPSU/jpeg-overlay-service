#include "image_processor/ImageCodec.h"
#include "image_processor/Exceptions.h"
#include <opencv2/imgcodecs.hpp>
#include <vector>
#include <filesystem>

namespace ImageProcessor {

    cv::Mat ImageCodec::Decode(const std::string& path)
    {
        cv::Mat img = cv::imread(path, cv::IMREAD_UNCHANGED);
        return img;
    }

    void ImageCodec::Encode(const cv::Mat& image, const std::string& path, int quality)
    {
        std::vector<int> params = {
            cv::IMWRITE_JPEG_QUALITY,
            quality
        };

        cv::imwrite(path, image, params);
    }

}