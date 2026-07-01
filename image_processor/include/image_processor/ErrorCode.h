#pragma once

#include <system_error>
#include <string>

namespace ImageProcessor
{
    enum class ImageError
    {
        None = 0,
        EmptyCaption,
        DecodeFailed,
        EncodeFailed
    };

    class ImageErrorCategory : public std::error_category
    {
    public:
        const char* name() const noexcept override
        {
            return "ImageProcessor";
        }

        std::string message(int ev) const override
        {
            switch (static_cast<ImageError>(ev))
            {
                case ImageError::None:
                    return "No error";
                case ImageError::EmptyCaption:
                    return "Caption text cannot be empty";
                case ImageError::DecodeFailed:
                    return "Failed to decode image";
                case ImageError::EncodeFailed:
                    return "Failed to encode image";
                default:
                    return "Unknown error";
            }
        }
    };

    inline const ImageErrorCategory& image_error_category()
    {
        static ImageErrorCategory instance;
        return instance;
    }

    inline std::error_code make_error_code(ImageError e)
    {
        return std::error_code(static_cast<int>(e), image_error_category());
    }
}

namespace std
{
    template<>
    struct is_error_code_enum<ImageProcessor::ImageError> : true_type {};
}
