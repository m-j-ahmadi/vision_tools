#include "image-processor.hpp"
#include <opencv2/imgproc.hpp> // for cv::cvtColor, resize, blur, etc.





cv::Mat BaseProcessor::process(const cv::Mat &image)
{
    return image.clone(); // Return a deep copy
}

cv::Mat GrayscaleProcessor::process(const cv::Mat &image)
{
    std::cout << "Im gray scale processor";
    cv::Mat result = wrapped_processor->process(image);
    cv::Mat gray_image;
    cv::cvtColor(result, gray_image, cv::COLOR_BGR2GRAY);
    return gray_image;
}

cv::Mat ResizeProcessor::process(const cv::Mat &image)
{
    cv::Mat result = wrapped_processor->process(image);
    cv::Mat resized_image;
    cv::resize(result, resized_image, cv::Size(width, height));
    return resized_image;
}

cv::Mat BlurProcessor::process(const cv::Mat &image)
{
    cv::Mat result = wrapped_processor->process(image);
    cv::Mat blurred_image;
    cv::GaussianBlur(result, blurred_image, cv::Size(kernel_size, kernel_size), 0);
    return blurred_image;
}
