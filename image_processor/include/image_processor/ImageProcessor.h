#pragma once

#include <memory>
#include <vector>
#include <string>

namespace ImageProcessor
{
    using Byte = unsigned char;

    class ImageProcessor
    {
    public:
        ImageProcessor();
        ~ImageProcessor();

        std::vector<Byte> Process(const std::vector<Byte>& imageBytes, const std::string& text);

    private:
        class Impl;
        std::unique_ptr<Impl> impl;
    };

}