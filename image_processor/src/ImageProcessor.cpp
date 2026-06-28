#include "image_processor/ImageProcessor.h"
#include "image_processor/ImageCodec.h"

namespace ImageProcessor 
{

    cv::Mat ImageProcessor::Process(const std::string& input, const std::string& text)
    {
        validator.ValidateImageFile(input);

        cv::Mat image = ImageCodec::Decode(input);

        validator.ValidateDecodedImage(image);

        return drawer.AddCaption(image, text);
    }

    void ImageProcessor::Save(const cv::Mat& image, const std::string& output, int quality)
    {
        validator.ValidateDecodedImage(image);

        ImageCodec::Encode(image, output, quality);
    }

}
