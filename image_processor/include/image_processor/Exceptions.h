#pragma once

#include <stdexcept>
#include <string>

namespace ImageProcessor
{

    class ImageException : public std::runtime_error
    {
    public:

        explicit ImageException(const std::string& message)
            : std::runtime_error(message) {}

        explicit ImageException(const char* message)
            : std::runtime_error(message) {}
    };

}