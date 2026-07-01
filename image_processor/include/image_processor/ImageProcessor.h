#pragma once

#include <memory>
#include <vector>
#include <string>
#include <system_error>
#include <opencv2/core.hpp>
#include "ErrorCode.h"

namespace ImageProcessor
{
    class ImageProcessor
    {
    public:
        ImageProcessor();
        ~ImageProcessor();

        std::error_code Process(
            const std::vector<uchar>& imageBytes,
            const std::string& text,
            std::vector<uchar>& result);

    private:
        class Impl;
        std::unique_ptr<Impl> impl;
    };

}